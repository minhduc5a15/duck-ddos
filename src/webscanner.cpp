#include "webscanner.h"
#include <curl/curl.h>
#include <thread>
#include <iostream>
#include <sstream>
#include <fstream>
#include <json/json.h>
#include "curl_utils.h"

std::atomic<bool> WebScanner::scanning(false);
std::string WebScanner::scan_result_;
ctpl::thread_pool WebScanner::pool(10);
std::unordered_map<std::string, std::string> WebScanner::scan_cache;

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

void WebScanner::scan_headers(const std::string &url, const std::string &custom_header, bool verify_ssl, const ProxyManager &proxy) {
    if (scan_cache.contains(url)) {
        scan_result_ = scan_cache[url];
        return;
    }

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
            long status_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
            std::ostringstream result;
            result << "URL: " << url << "\nStatus: " << status_code << "\nHeaders:\n" << response_headers;
            scan_result_ = result.str();
            scan_cache[url] = scan_result_;
            std::cout << result.str() << std::endl;

            // Export to JSON
            export_to_json("scan_result.json", url, status_code, response_headers);
        }
        else {
            scan_result_ = "Scan failed: " + std::string(curl_easy_strerror(res));
            std::cerr << scan_result_ << std::endl;
        }
        if (headers) curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}

void WebScanner::scan_security_headers(const std::string &url, const std::string &custom_header, bool verify_ssl, const ProxyManager &proxy) {
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
            std::ostringstream result;
            result << "URL: " << url << "\nSecurity Headers:\n";
            if (response_headers.find("Content-Security-Policy") == std::string::npos) {
                result << "Missing: Content-Security-Policy\n";
            }
            if (response_headers.find("X-Frame-Options") == std::string::npos) {
                result << "Missing: X-Frame-Options\n";
            }
            if (response_headers.find("Strict-Transport-Security") == std::string::npos) {
                result << "Missing: Strict-Transport-Security\n";
            }
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

void WebScanner::scan_files_and_directories(const std::string &url, const ProxyManager &proxy) {
    std::vector<std::string> common_paths = {"/robots.txt", "/.git/", "/.env", "/admin/", "/backup/"};
    for (const auto &path: common_paths) {
        std::string full_url = url + path;
        CURL *curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
            curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
            if (proxy.is_enabled()) {
                curl_easy_setopt(curl, CURLOPT_PROXY, proxy.get_random_proxy().c_str());
            }
            const CURLcode res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                long status_code;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
                if (status_code == 200) {
                    scan_result_ += "Found: " + full_url + "\n";
                }
            }
            curl_easy_cleanup(curl);
        }
    }
}

void WebScanner::start(const std::string &url, const std::string &scan_type, const int threads,
                       const std::string &custom_header, bool verify_ssl, const ProxyManager &proxy) {
    scanning = true;
    scan_result_.clear();
    for (int i = 0; i < threads; ++i) {
        if (scan_type == "Headers") {
            pool.push([url, custom_header, verify_ssl, proxy](int) {
                scan_headers(url, custom_header, verify_ssl, proxy);
            });
        }
        else if (scan_type == "Security Headers") {
            pool.push([url, custom_header, verify_ssl, proxy](int) {
                scan_security_headers(url, custom_header, verify_ssl, proxy);
            });
        }
        else if (scan_type == "Files and Directories") {
            pool.push([url, proxy](int) {
                scan_files_and_directories(url, proxy);
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

void WebScanner::export_to_csv(const std::string &filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "URL,Status,Headers\n";
        file << scan_result_;
        file.close();
    }
}

void WebScanner::export_to_json(const std::string &filename, const std::string &url, long status_code, const std::string &response_headers) {
    Json::Value root;
    root["URL"] = url;
    root["Status"] = status_code;
    root["Headers"] = response_headers;

    std::ofstream file(filename);
    if (file.is_open()) {
        file << root.toStyledString();
        file.close();
    }
}

void WebScanner::set_request_rate(int rate) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000 / rate));
}
