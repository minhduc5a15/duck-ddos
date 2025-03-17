#include "proxy.h"
#include <chrono>
#include "curl_utils.h"
#include <curl/curl.h>

ProxyManager::ProxyManager(const std::string &proxy)
    : proxy_address_(proxy), enabled(!proxy.empty()),
      rng(static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count())) {}

void ProxyManager::set_proxy(const std::string &proxy) {
    proxy_address_ = proxy;
    proxy_list_.clear();
    enabled = !proxy.empty();
}

void ProxyManager::set_proxy_list(const std::vector<std::string> &proxies) {
    proxy_list_ = proxies;
    proxy_address_.clear();
    enabled = !proxies.empty();
}

std::string ProxyManager::get_proxy() const {
    return proxy_address_;
}

std::string ProxyManager::get_random_proxy() const {
    if (proxy_list_.empty()) return proxy_address_;
    if (proxy_list_.size() == 1) return proxy_list_[0];
    std::uniform_int_distribution<size_t> dist(0, proxy_list_.size() - 1);
    return proxy_list_[dist(rng)];
}

bool ProxyManager::test_proxy(const std::string &proxy) const {
    CURL *curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://google.com");
        curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlUtils::write_callback);
        const CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        return res == CURLE_OK;
    }
    return false;
}

bool ProxyManager::is_enabled() const {
    return enabled;
}

void ProxyManager::enable(const bool enabled) {
    this->enabled = enabled;
}

size_t ProxyManager::proxy_count() const {
    return proxy_list_.empty() ? (proxy_address_.empty() ? 0 : 1) : proxy_list_.size();
}
