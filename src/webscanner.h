#ifndef WEBSCANNER_H
#define WEBSCANNER_H

#include <string>
#include "proxy.h"
#include "cptl.h"

class WebScanner {
public:
    static void start(const std::string &url, const std::string &scanType, int threads,
                      const std::string &custom_header = "", bool verify_ssl = true, const ProxyManager &proxy = ProxyManager());

    static void stop();

    static std::string get_scan_result();

private:
    static void scan_headers(const std::string &url, const std::string &custom_header, bool verify_ssl, const ProxyManager &proxy);

    static std::string get_random_user_agent();

    static std::atomic<bool> scanning;
    static std::string scan_result_;
    static ctpl::thread_pool pool;
};

#endif // WEBSCANNER_H
