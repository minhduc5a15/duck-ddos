#include "proxy.h"
#include <chrono>

ProxyManager::ProxyManager(const std::string& proxy)
    : proxyAddress(proxy), enabled(!proxy.empty()),
      rng(static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count())) {}

void ProxyManager::setProxy(const std::string& proxy) {
    proxyAddress = proxy;
    proxyList.clear();
    enabled = !proxy.empty();
}

void ProxyManager::setProxyList(const std::vector<std::string>& proxies) {
    proxyList = proxies;
    proxyAddress.clear();
    enabled = !proxies.empty();
}

std::string ProxyManager::getProxy() const {
    return proxyAddress;
}

std::string ProxyManager::getRandomProxy() const {
    if (proxyList.empty()) return proxyAddress;
    if (proxyList.size() == 1) return proxyList[0];
    std::uniform_int_distribution<size_t> dist(0, proxyList.size() - 1);
    return proxyList[dist(rng)];
}

bool ProxyManager::isEnabled() const {
    return enabled;
}

void ProxyManager::enable(const bool enabled) {
    this->enabled = enabled;
}

size_t ProxyManager::proxyCount() const {
    return proxyList.empty() ? (proxyAddress.empty() ? 0 : 1) : proxyList.size();
}