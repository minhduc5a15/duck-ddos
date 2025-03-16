#ifndef PROXY_H
#define PROXY_H

#include <string>
#include <vector>
#include <random>

class ProxyManager {
public:
    explicit ProxyManager(const std::string& proxy = "");
    void setProxy(const std::string& proxy);
    void setProxyList(const std::vector<std::string>& proxies);
    std::string getProxy() const; // Lấy proxy cố định
    std::string getRandomProxy() const; // Lấy proxy ngẫu nhiên
    bool isEnabled() const;
    void enable(bool enabled);
    size_t proxyCount() const;

private:
    std::string proxyAddress; // Proxy đơn
    std::vector<std::string> proxyList; // Danh sách proxy
    bool enabled;
    mutable std::mt19937 rng; // Random engine
};

#endif // PROXY_H