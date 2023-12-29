#include <curl/curl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "api.h"
#include "config.h"
#include "log.h"

#define LCAPI_URLMAXLENGTH 256

static void error(char * fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   char prepend[] = "ERROR: ";
   char * newfmt = malloc(strlen(prepend) + strlen(fmt) + 1);
   if(newfmt) {
      sprintf(newfmt, "%s%s", prepend, fmt);
      vfprintf(stderr, newfmt, args);
      //log_error(newfmt, args);
      free(newfmt);
   }
   else {
      vfprintf(stderr, fmt, args);
      //log_error(fmt, args);
   }
   exit(1);
}

void lc_curlinit()
{
   if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
      error("libcurl initialization failed");
   }
   if (atexit(curl_global_cleanup) != 0) {
      curl_global_cleanup();
      error("couldn't register libcurl cleanup");
   }
   log_debug("Setup libcurl!");
}

// Return a CURL object pointing to the given url with GET, which you need to cleanup
CURL * lc_curlget(char * url)
{
   CURL * curl = curl_easy_init(); 
   if(!curl) {
      error("Couldn't make curl object to %s", url);
   }

   curl_easy_setopt(curl, CURLOPT_URL, url);
   curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");

   return curl;
}

CURL * lc_curlget_api(char * endpoint, struct LowcapiConfig * config)
{
   char url[LCAPI_URLMAXLENGTH];
   snprintf(url, sizeof(url), "%s/%s", config->api, endpoint);
   return lc_curlget(url);
}

CURL * lc_curlget_small(char * endpoint, struct LowcapiConfig * config)
{
   char url[LCAPI_URLMAXLENGTH];
   snprintf(url, sizeof(url), "%s/small/%s", config->api, endpoint);
   return lc_curlget(url);
}

// Taken from chatgpt, oof
size_t lc_curl_writecallback(void *contents, size_t size, size_t nmemb, char **output) {
    size_t totalSize = size * nmemb;
    *output = (char*)realloc(*output, totalSize + 1); // +1 for null-terminator
    if (*output) {
        memcpy(*output, contents, totalSize);
        (*output)[totalSize] = '\0'; // Null-terminate the string
    }
    return totalSize;
}

void lc_curl_setupcallback(CURL * curl, char ** response)
{
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, lc_curl_writecallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
}

char * lc_getany(char * endpoint, struct LowcapiConfig * config, int fail_critical)
{
   //Make an initial request to the status endpoint
   CURL * statuscurl = lc_curlget_api("status", config);
   char * response = NULL;
   lc_curl_setupcallback(statuscurl, &response);

   CURLcode statusres = curl_easy_perform(statuscurl);
   curl_easy_cleanup(statuscurl);

   if(fail_critical)
   {
      if (statusres != CURLE_OK) {
         error("Couldn't get %s endpoint - %s", endpoint, curl_easy_strerror(statusres));
      }
      if (!response) {
         error("Failed to fetch response data for %s endpoint", endpoint);
      }
   }

   return response;
}

