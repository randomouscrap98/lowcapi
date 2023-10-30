#include <curl/curl.h>
#include <stdlib.h>

#include "api.h"
#include "config.h"

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

