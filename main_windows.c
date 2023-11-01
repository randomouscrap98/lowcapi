#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

//Deps
#include "csv.h"
#include "log.h"

//Our own crap
#include "config.h"
#include "api.h"

static void error(const char* msg, const char* msg1)
{
   fprintf(stderr, "ERROR: %s%s\n", msg, msg1?msg1:"");
   exit(1);
}

int main(int argc, char * argv[])
{
	printf("HELLO??\n");
   struct LowcapiConfig config = lc_read_config(argc > 1 ? argv[1] : NULL);

   lc_setup_logging(&config);
   log_info("Program started");
   lc_log_config(&config);

   if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
      error("libcurl initialization failed", NULL);
   }
   if (atexit(curl_global_cleanup) != 0) {
      error("couldn't register libcurl cleanup", NULL);
   }
	printf("DID CURL SETUP??\n");

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
      log_debug("Status response:\n%s\n", response);
      free(response); 
   } else {
      error("Failed to fetch response data", NULL);
   }

   return 0;
}

