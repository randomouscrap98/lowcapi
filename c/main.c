#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include <stdbool.h>

//Our own crap
#include "mycsv.h"
#include "api.h"
#include "lcutils.h"

#define SMALLINPUTLEN 100


// Retrieve the environment variables we will generally use
// for all endpoints
void get_base_env(CapiValues * capi, bool token_required)
{
   // Read a bunch of environment variables, just so we have them. Yes yes
   // wasteful if you don't need them blah blah blah
   char * CAPI_URL = getenv("CAPI_URL");
   char * CAPI_TOKEN = getenv("CAPI_TOKEN");

   int capilen = CAPI_URL ? strlen(CAPI_URL) : 0;
   int tokenlen = CAPI_TOKEN? strlen(CAPI_TOKEN) : 0;

   if(!capilen) { error("Must provide CAPI_URL environment variable!"); }
   if(capilen > LC_URLPARTLENGTH) { error("URL too long! Max: %d", LC_URLPARTLENGTH); }

   strcpy(capi->api, CAPI_URL);

   if(tokenlen) {
      if(tokenlen > LC_TOKENMAXLENGTH) {
         error("Token too long! Max: %d", LC_TOKENMAXLENGTH);
      }
      strcpy(capi->token, CAPI_TOKEN);
   }
   else {
      if(token_required) {
         error("Must provide CAPI_TOKEN environment variable!");
      }
      capi->token[0] = 0;
   }
}


// ----------------------------
//       Main                  
// ----------------------------


int main(int argc, char * argv[])
{
   if(argc < 2) {
      error("Must provide the action! Usage:\n%s\n%s\n%s\n%s",
            " lowcapi auth",
            " lowcapi search",
            " lowcapi send <roomid>",
            " lowcapi listen <roomid>"
           );
   }

   lc_curlinit();

   CapiValues capi;

   if(!strcmp(argv[1], "auth")) {
      get_base_env(&capi, false);
   }
   else if(!strcmp(argv[1], "search")) {
      get_base_env(&capi, false);
   }
   else if(!strcmp(argv[1], "send")) {
      get_base_env(&capi, true);
   }
   else if(!strcmp(argv[1], "lilsten")) {
      get_base_env(&capi, true);
   }
   else {
      error("Unknown command: %s", argv[1]);
   }


   return 0;
}


