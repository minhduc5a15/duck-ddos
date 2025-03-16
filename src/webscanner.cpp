#include "webscanner.h"
#include <curl/curl.h>
#include <thread>
#include <iostream>
#include <sstream>

std::atomic<bool> WebScanner::scanning(false);
std::string WebScanner::scanResult;
ctpl::thread_pool WebScanner::pool(50);

std::string WebScanner::getRandomUserAgent() {
    static const std::vector<std::string> userAgents = {
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.0 Safari/605.1.15",
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/88.0.4324.96 Safari/537.36"
    };
    static std::mt19937 rng(static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<size_t> dist(0, userAgents.size() - 1);
    return userAgents[dist(rng)];
}

void WebScanner::scanHeaders(const std::string &url, const std::string &customHeader, const bool verifySSL, const ProxyManager &proxy) {
    CURL *curl = curl_easy_init();
    curl_slist *headers = nullptr;
    std::string responseHeaders;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlUtils::writeCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, CurlUtils::headerCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &responseHeaders);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, verifySSL ? 1L : 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, verifySSL ? 2L : 0L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, getRandomUserAgent().c_str());
        if (!customHeader.empty()) {
            headers = curl_slist_append(nullptr, customHeader.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }
        if (proxy.isEnabled()) {
            curl_easy_setopt(curl, CURLOPT_PROXY, proxy.getRandomProxy().c_str());
        }
        const CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            long statusCode;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
            std::ostringstream result;
            result << "URL: " << url << "\nStatus: " << statusCode << "\nHeaders:\n" << responseHeaders;
            scanResult = result.str();
            std::cout << result.str() << std::endl;
        }
        else {
            scanResult = "Scan failed: " + std::string(curl_easy_strerror(res));
            std::cerr << scanResult << std::endl;
        }
        if (headers) curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}

void WebScanner::start(const std::string &url, const std::string &scanType, const int threads,
                       const std::string &customHeader, bool verifySSL, const ProxyManager &proxy) {
    scanning = true;
    scanResult.clear();
    for (int i = 0; i < threads; ++i) {
        if (scanType == "Headers") {
            pool.push([url, customHeader, verifySSL, proxy](int) {
                scanHeaders(url, customHeader, verifySSL, proxy);
            });
        }
    }
}

void WebScanner::stop() {
    scanning = false;
}

std::string WebScanner::getScanResult() {
    return scanResult;
}
