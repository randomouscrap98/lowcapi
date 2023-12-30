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
#include "screen.h"


int main(int argc, char * argv[])
{
   struct LowcapiConfig config = lc_read_config(argc > 1 ? argv[1] : NULL);

   lc_setup_logging(&config);
   log_info("Program started");
   lc_log_config(&config);

   lc_curlinit();
   lc_setup_screen();

   printw("Checking connection to %s...\n", config.api);
   refresh();

   //Make an initial request to the status endpoint
   struct HttpResponse * response = lc_getany("status", &config, 1);
   log_debug("API Status response:\n%s\n", response->response);
   print_color(LCSCL_OK, "Connection OK! [%ld]\n", response->status);
   lc_freeresponse(response);
   refresh();


   char * token = lc_gettoken(&config);
   const int inputlength = 100;
   char username[inputlength];
   char password[inputlength];

   if(!token) {
      print_color(LCSCL_WARN, "No token file found, please login\n");
      refresh();
      while(!token) {
         printw("Username: ");
         lc_getinput_simple(username, inputlength);
         printw("\nPassword: ");
         lc_getpass_simple(password, inputlength);
         printw("\nLogging in...\n");
         refresh();
         char * output = NULL;
         if(lc_consumeresponse(lc_login(username, password, &config, 0), &output))
         {
            token = output;
            printw("Token: %s\n", token);
         }
         else if(output)
         {
            print_color(LCSCL_WARN, "Error: %s\n", output);
            free(output);
         }
         else
         {
            print_color(LCSCL_WARN, "Error: UNKNOWN\n");
         }
         refresh();
      }
   }

   log_info("Program end");
   printw("Program end\n");
   refresh();

   getch();

   return 0;
}

