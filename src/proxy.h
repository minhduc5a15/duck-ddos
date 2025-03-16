#ifndef PROXY_H
#define PROXY_H

#include <string>

class ProxyManager {
public:
    explicit ProxyManager(const std::string& proxy = "");
    void setProxy(const std::string& proxy);
    std::string getProxy() const;
    bool isEnabled() const;
    void enable(bool enabled);

private:
    std::string proxyAddress; // Lưu IP:Port (e.g., "103.221.222.222:8080")
    bool enabled; // Trạng thái bật/tắt proxy
};

#endif // PROXY_H