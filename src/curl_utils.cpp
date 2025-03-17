#include "curl_utils.h"

size_t CurlUtils::write_callback(void *contents, const size_t size, const size_t nmemb, void *userp) {
    return size * nmemb;
}

size_t CurlUtils::header_callback(const char *buffer, const size_t size, const size_t nitems, void *userdata) {
    const auto headers = static_cast<std::string *>(userdata);
    headers->append(buffer, size * nitems);
    return size * nitems;
}
