#ifndef CURL_UTILS_H
#define CURL_UTILS_H

#include <string>
#include <curl/curl.h>

class CurlUtils {
public:
    static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp);

    static size_t header_callback(const char *buffer, size_t size, size_t nitems, void *userdata);

    void operator()(CURL *curl) const {
        if (curl) curl_easy_cleanup(curl);
    }

    struct curl_deleter {
        void operator()(CURL *curl) const { if (curl) curl_easy_cleanup(curl); }
    };

    struct curl_slist_deleter {
        void operator()(curl_slist *slist) const { if (slist) curl_slist_free_all(slist); }
    };
};

#endif // CURL_UTILS_H
