#ifndef PROXY_H
#define PROXY_H

#include <string>
#include <vector>
#include <random>
#include "curl_utils.h"

class ProxyManager {
public:
    explicit ProxyManager(const std::string &proxy = "");

    void setProxy(const std::string &proxy);

    void setProxyList(const std::vector<std::string> &proxies);

    std::string getProxy() const;

    std::string getRandomProxy() const;

    bool testProxy(const std::string &proxy) const;

    bool isEnabled() const;

    void enable(bool enabled);

    size_t proxyCount() const;

private:
    std::string proxyAddress;
    std::vector<std::string> proxyList;
    bool enabled;
    mutable std::mt19937 rng;
};

#endif // PROXY_H
