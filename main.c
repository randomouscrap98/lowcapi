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
//#include "csv.h"
#include "log.h"

//Our own crap
#include "config.h"
#include "api.h"
#include "screen.h"

#define SMALLINPUTLEN 100

// Ask user for new login info. Writes login info to token file.
void newlogin(struct LowcapiConfig * config)
{
   char username[SMALLINPUTLEN];
   char password[SMALLINPUTLEN];

   while(1) {
      printw("Username: ");
      lc_getinput_simple(username, SMALLINPUTLEN);
      printw("\nPassword: ");
      lc_getpass_simple(password, SMALLINPUTLEN);
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

long roomsearch(struct LowcapiConfig * config)
{
   long roomid = 0;
   char roomname[SMALLINPUTLEN];

   struct HttpRequest request;
   lc_initrequest(&request, "small/search", config);

   while(!roomid)
   {
      printw("Search for a room or enter an ID: ");
      refresh();
      lc_getinput_simple(roomname, SMALLINPUTLEN);
      printw("\n");
      refresh();

      roomid = atoi(roomname);
      struct RequestValue * values = NULL;

      if(roomid)
         values = lc_addvalue(NULL, "id", roomname);
      else
         values = lc_addvalue(NULL, "search", roomname);

      char * output;

      if(lc_consumeresponse(lc_getapi(&request, values), &output))
      {
         //Need to parse the lines and output each one.
      }
      else
      {
         print_color(LCSCL_ERR, "Search error: %s\n", output ? output : "UNKNOWN");
      }

      lc_freeallvalues(values);
   }

   return roomid;
}

int checkconnection(struct LowcapiConfig * config)
{
   printw("Checking connection to %s...\n", config->api);
   refresh();

   //Make an initial request to the status endpoint
   struct HttpRequest request;
   lc_initrequest(&request, "status", config);
   request.fail_critical = 1;

   struct HttpResponse * response = lc_getapi(&request, NULL);
   log_debug("API Status response:\n%s", response->response);

   int result = lc_responseok(response);
   if(result) { print_color(LCSCL_OK, "Connection OK! [%ld]\n", response->status); }
   else { print_color(LCSCL_ERR, "Connection failed! [%ld]\n", response->status); }
   refresh();

   lc_freeresponse(response);

   return result;
}


// ----------------------------
//       Main                  
// ----------------------------

int main(int argc, char * argv[])
{
   struct LowcapiConfig config = lc_read_config(argc > 1 ? argv[1] : NULL);

   lc_setup_logging(&config);
   log_info("Program started");
   lc_log_config(&config);

   lc_curlinit();
   lc_setup_screen();

   if(checkconnection(&config))
   {
      char * token = lc_gettoken(&config);

      while(!token)
      {
         print_color(LCSCL_WARN, "No token file found, please login\n");
         refresh();
         newlogin(&config);
         token = lc_gettoken(&config);
      }

      printw("Token file found, testing login...\n");
      refresh();

      struct MeResponse me = lc_getme(token, &config);

      while(!me.userid)
      {
         print_color(LCSCL_WARN, "Token file invalid, please login again\n");
         refresh();
         newlogin(&config);
         free(token); //Free the old token
         token = lc_gettoken(&config);
         me = lc_getme(token, &config);
      }

      printw("Logged in as %s (%ld)!\n", me.username, me.userid);
      refresh();

      long roomid = roomsearch(&config);
   }

   log_info("Program end");
   printw("Program end\n");
   refresh();

   getch();

   return 0;
}


