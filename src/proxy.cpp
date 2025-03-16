#include "proxy.h"

ProxyManager::ProxyManager(const std::string& proxy) : proxyAddress(proxy), enabled(!proxy.empty()) {}

void ProxyManager::setProxy(const std::string& proxy) {
    proxyAddress = proxy;
    enabled = !proxy.empty();
}

std::string ProxyManager::getProxy() const {
    return proxyAddress;
}

bool ProxyManager::isEnabled() const {
    return enabled;
}

void ProxyManager::enable(const bool enabled) {
    this->enabled = enabled;
}