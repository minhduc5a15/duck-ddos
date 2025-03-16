#ifndef CURL_UTILS_H
#define CURL_UTILS_H

#include <string>

class CurlUtils {
public:
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
    static size_t headerCallback(const char* buffer, size_t size, size_t nitems, void* userdata); // Thêm hàm này
};

#endif // CURL_UTILS_H