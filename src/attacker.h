#ifndef ATTACKER_H
#define ATTACKER_H

#include <atomic>
#include <string>
#include "proxy.h"

class Attacker {
public:
    static void start(const std::string& url, const std::string& type, int threads, int requests = 100, const ProxyManager& proxy = ProxyManager());
    static void stop();
    static void pause();
    static bool isPaused();
    static bool isAttacking();
    static int getSuccessfulRequests();
    static int getFailedRequests();

private:
    static void httpFlood(const std::string& url, int requests, const ProxyManager& proxy);
    static void slowloris(const std::string& url, const ProxyManager& proxy);
    static std::string getRandomUserAgent();
    static std::atomic<bool> attacking;
    static std::atomic<bool> paused;
    static std::atomic<int> successfulRequests;
    static std::atomic<int> failedRequests;
};

#endif // ATTACKER_H