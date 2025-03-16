#include "curl_utils.h"

size_t CurlUtils::writeCallback(void* contents, const size_t size, const size_t nmemb, void* userp) {
    return size * nmemb;
}