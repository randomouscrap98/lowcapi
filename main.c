#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

//You're SUPPOSED to be able to use just curses.h on any system and it'll be
//portable, but in practice it just doesn't work like that... blegh
#ifdef BUILDWINDOWS
#include <pdcurses.h>
#else
#include <ncurses.h>
#endif

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
   struct LowcapiConfig config = lc_read_config(argc > 1 ? argv[1] : NULL);

   lc_setup_logging(&config);
   log_info("Program started");
   lc_log_config(&config);

   lc_curlinit();

   printf("Checking connection to %s...\n", config.api);

   //Make an initial request to the status endpoint
   char * response = lc_getany("status", &config, 1);
   log_debug("API Status response:\n%s\n", response);
   free(response);

   printf("Connection OK!\n");


   return 0;
}

