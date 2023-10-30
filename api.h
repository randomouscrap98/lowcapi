#ifndef LC_API_GUARD
#define LC_API_GUARD

#include <curl/curl.h>
#include "config.h"

// Return a CURL object pointing to the given url with GET, which you need to cleanup
CURL * curlget(char * url);
CURL * curlget_api(char * endpoint, struct LowcapiConfig * config);
CURL * curlget_small(char * endpoint, struct LowcapiConfig * config);

size_t curl_writecallback(void *contents, size_t size, size_t nmemb, char **output);
void curl_setupcallback(CURL * curl, char * response);

#endif
