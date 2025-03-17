#ifndef WEBSCANNER_H
#define WEBSCANNER_H

#include <string>
#include <atomic>
#include <unordered_map>
#include "proxy.h"
#include "cptl.h"

class WebScanner {
public:
    static void start(const std::string &url, const std::string &scan_type, int threads,
                      const std::string &custom_header = "", bool verify_ssl = true, const ProxyManager &proxy = ProxyManager());

    static void stop();

    static std::string get_scan_result();

    static void export_to_csv(const std::string &filename);
    static void export_to_json(const std::string &filename, const std::string &url, long status_code, const std::string &response_headers);

    static void set_request_rate(int rate);

private:
    static void scan_headers(const std::string &url, const std::string &custom_header, bool verify_ssl, const ProxyManager &proxy);
    static void scan_security_headers(const std::string &url, const std::string &custom_header, bool verify_ssl, const ProxyManager &proxy);
    static void scan_files_and_directories(const std::string &url, const ProxyManager &proxy);

    static std::string get_random_user_agent();

    static std::atomic<bool> scanning;
    static std::string scan_result_;
    static ctpl::thread_pool pool;
    static std::unordered_map<std::string, std::string> scan_cache;
};

#endif // WEBSCANNER_H