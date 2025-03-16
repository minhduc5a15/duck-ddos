#ifndef CURL_UTILS_H
#define CURL_UTILS_H

#include <string>

class CurlUtils {
public:
    static size_t writeCallback(void *, size_t size, size_t nmemb, void *);
};

#endif // CURL_UTILS_H
