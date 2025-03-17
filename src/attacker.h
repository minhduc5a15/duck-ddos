#ifndef ATTACKER_H
#define ATTACKER_H

#include <atomic>
#include <string>
#include "proxy.h"
#include "cptl.h"

class Attacker {
public:
    static void start(const std::string &url, const std::string &type, int threads, int requests = 100, int rps = 100,
                      const std::string &custom_header = "", bool verify_ssl = true, const ProxyManager &proxy = ProxyManager());

    static void stop();

    static void pause();

    static bool is_paused();

    static bool is_attacking();

    static int get_successful_requests();

    static int get_failed_requests();

private:
    static void http_flood(const std::string &url, int requests, int rps, const std::string &custom_header, bool verify_ssl, const ProxyManager &proxy);

    static void slowloris(const std::string &url, const ProxyManager &proxy);

    static std::string get_random_user_agent();

    static std::atomic<bool> attacking;
    static std::atomic<bool> paused;
    static std::atomic<int> successful_requests_;
    static std::atomic<int> failed_requests_;
    static ctpl::thread_pool pool;
};

#endif // ATTACKER_H
