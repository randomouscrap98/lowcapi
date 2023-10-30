#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

#include "csv.h"
#include "config.h"
#include "api.h"

static void error(const char* msg, const char* msg1)
{
   fprintf(stderr, "ERROR: %s%s\n", msg, msg1?msg1:"");
   exit(1);
}

int main(int argc, char * argv[])
{
   if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
      error("libcurl initialization failed", NULL);
   }
   if (atexit(curl_global_cleanup) != 0) {
      error("couldn't register libcurl cleanup", NULL);
   }

   struct LowcapiConfig config = lc_read_config(argc > 1 ? argv[1] : NULL);
   printf("api: %s\n", config.api);
   printf("initpull: %d\n", config.initpull);

   //Make an initial request to the status endpoint
   CURL * statuscurl = curlget_api("status", &config);
   char * response = NULL;
   curl_setupcallback(statuscurl, &response);

   CURLcode statusres = curl_easy_perform(statuscurl);
   if (statusres != CURLE_OK) {
      curl_easy_cleanup(statuscurl);
      error("Couldn't get status endpoint - ", curl_easy_strerror(statusres));
   }

   if (response) {
      printf("Status response:\n%s\n", response);
      free(response); 
   } else {
      error("Failed to fetch response data", NULL);
   }

   return 0;
}

