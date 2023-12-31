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


// Ask user for new login info. Writes login info to token file.
void newlogin(struct LowcapiConfig * config)
{
   const int inputlength = 100;
   char username[inputlength];
   char password[inputlength];

   while(1) {
      printw("Username: ");
      lc_getinput_simple(username, inputlength);
      printw("\nPassword: ");
      lc_getpass_simple(password, inputlength);
      printw("\nLogging in...\n");
      refresh();
      char * output = NULL;
      if(lc_consumeresponse(lc_login(username, password, config), &output))
      {
         print_color(LCSCL_OK, "Token received, writing to file\n");
         lc_storetoken(config, output);
         break;
      }
      else if(output)
      {
         print_color(LCSCL_WARN, "Error: %s\n", output);
      }
      else
      {
         print_color(LCSCL_WARN, "Error: UNKNOWN\n");
      }
      free(output); //Apparently it's safe to call free on null
      refresh();
   }
}

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
   struct HttpRequest request;
   request.fail_critical = 1;
   request.config = &config;
   request.token[0] = 0;
   sprintf(request.endpoint, "status");

   struct HttpResponse * response = lc_getapi(&request, NULL);
   log_debug("API Status response:\n%s\n", response->response);
   print_color(LCSCL_OK, "Connection OK! [%ld]\n", response->status);
   lc_freeresponse(response);
   refresh();

   char * token = lc_gettoken(&config);

   while(!token)
   {
      print_color(LCSCL_WARN, "No token file found, please login\n");
      refresh();
      newlogin(&config);
      token = lc_gettoken(&config);
   }

   printw("Token file found, testing login...\n");

   log_info("Program end");
   printw("Program end\n");
   refresh();

   getch();

   return 0;
}


