#ifndef WEBSCANNER_H
#define WEBSCANNER_H

#include <string>
#include "proxy.h"
#include "cptl.h"

class WebScanner {
public:
    static void start(const std::string &url, const std::string &scanType, int threads,
                      const std::string &customHeader = "", bool verifySSL = true, const ProxyManager &proxy = ProxyManager());

    static void stop();

    static std::string getScanResult();

private:
    static void scanHeaders(const std::string &url, const std::string &customHeader, bool verifySSL, const ProxyManager &proxy);

    static std::string getRandomUserAgent();

    static std::atomic<bool> scanning;
    static std::string scanResult;
    static ctpl::thread_pool pool;
};

#endif // WEBSCANNER_H
