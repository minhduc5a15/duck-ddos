#ifndef PROXY_H
#define PROXY_H

#include <string>
#include <vector>
#include <random>

class ProxyManager {
public:
    explicit ProxyManager(const std::string &proxy = "");

    void set_proxy(const std::string &proxy);

    void set_proxy_list(const std::vector<std::string> &proxies);

    std::string get_proxy() const;

    std::string get_random_proxy() const;

    bool test_proxy(const std::string &proxy) const;

    bool is_enabled() const;

    void enable(bool enabled);

    size_t proxy_count() const;

private:
    std::string proxy_address_;
    std::vector<std::string> proxy_list_;
    bool enabled;
    mutable std::mt19937 rng;
};

#endif // PROXY_H
