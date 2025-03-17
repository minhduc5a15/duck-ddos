#include "webscanner.h"
#include <curl/curl.h>
#include <thread>
#include <iostream>
#include <sstream>

#include "curl_utils.h"

std::atomic<bool> WebScanner::scanning(false);
std::string WebScanner::scan_result_;
ctpl::thread_pool WebScanner::pool(50);

std::string WebScanner::get_random_user_agent() {
    static const std::vector<std::string> userAgents = {
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.0 Safari/605.1.15",
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/88.0.4324.96 Safari/537.36"
    };
    static std::mt19937 rng(static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<size_t> dist(0, userAgents.size() - 1);
    return userAgents[dist(rng)];
}

void WebScanner::scan_headers(const std::string &url, const std::string &custom_header, const bool verify_ssl, const ProxyManager &proxy) {
    CURL *curl = curl_easy_init();
    curl_slist *headers = nullptr;
    std::string response_headers;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlUtils::write_callback);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, CurlUtils::header_callback);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response_headers);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, verify_ssl ? 1L : 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, verify_ssl ? 2L : 0L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, get_random_user_agent().c_str());
        if (!custom_header.empty()) {
            headers = curl_slist_append(nullptr, custom_header.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }
        if (proxy.is_enabled()) {
            curl_easy_setopt(curl, CURLOPT_PROXY, proxy.get_random_proxy().c_str());
        }
        const CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            long statusCode;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
            std::ostringstream result;
            result << "URL: " << url << "\nStatus: " << statusCode << "\nHeaders:\n" << response_headers;
            scan_result_ = result.str();
            std::cout << result.str() << std::endl;
        }
        else {
            scan_result_ = "Scan failed: " + std::string(curl_easy_strerror(res));
            std::cerr << scan_result_ << std::endl;
        }
        if (headers) curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}

void WebScanner::start(const std::string &url, const std::string &scanType, const int threads,
                       const std::string &custom_header, bool verify_ssl, const ProxyManager &proxy) {
    scanning = true;
    scan_result_.clear();
    for (int i = 0; i < threads; ++i) {
        if (scanType == "Headers") {
            pool.push([url, custom_header, verify_ssl, proxy](int) {
                scan_headers(url, custom_header, verify_ssl, proxy);
            });
        }
    }
}

void WebScanner::stop() {
    scanning = false;
}

std::string WebScanner::get_scan_result() {
    return scan_result_;
}
