#include <curl/curl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "api.h"
#include "config.h"

#define LCAPI_URLMAXLENGTH 256

static void error(char* msg, char* msg1)
{
   fprintf(stderr, "ERROR: %s%s\n", msg, msg1?msg1:"");
   exit(1);
}

// Return a CURL object pointing to the given url with GET, which you need to cleanup
CURL * curlget(char * url)
{
   CURL * curl = curl_easy_init(); 
   if(!curl) {
      error("Couldn't make curl object to - ", url);
   }

   curl_easy_setopt(curl, CURLOPT_URL, url);
   curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");

   return curl;
}

CURL * curlget_api(char * endpoint, struct LowcapiConfig * config)
{
   char url[LCAPI_URLMAXLENGTH];
   snprintf(url, sizeof(url), "%s/%s", config->api, endpoint);
   return curlget(url);
}

CURL * curlget_small(char * endpoint, struct LowcapiConfig * config)
{
   char url[LCAPI_URLMAXLENGTH];
   snprintf(url, sizeof(url), "%s/small/%s", config->api, endpoint);
   return curlget(url);
}

// Taken from chatgpt, oof
size_t curl_writecallback(void *contents, size_t size, size_t nmemb, char **output) {
    size_t totalSize = size * nmemb;
    *output = (char*)realloc(*output, totalSize + 1); // +1 for null-terminator
    if (*output) {
        memcpy(*output, contents, totalSize);
        (*output)[totalSize] = '\0'; // Null-terminate the string
    }
    return totalSize;
}

void curl_setupcallback(CURL * curl, char * response)
{
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writecallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
}
