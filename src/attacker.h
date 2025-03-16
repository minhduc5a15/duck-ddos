#ifndef ATTACKER_H
#define ATTACKER_H

#include <atomic>
#include <string>
#include "proxy.h" // Include ProxyManager

class Attacker {
public:
    static void start(const std::string& url, const std::string& type, int threads, int requests = 100, const ProxyManager& proxy = ProxyManager()); // Truyền ProxyManager
    static void stop();
    static void pause();
    static bool isPaused();
    static bool isAttacking();

private:
    static void httpFlood(const std::string& url, int requests, const ProxyManager& proxy); // Truyền ProxyManager
    static void slowloris(const std::string& url, const ProxyManager& proxy); // Truyền ProxyManager
    static std::atomic<bool> attacking;
    static std::atomic<bool> paused;
};

#endif // ATTACKER_H