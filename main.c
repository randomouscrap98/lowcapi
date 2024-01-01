#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include <locale.h>

//You're SUPPOSED to be able to use just curses.h on any system and it'll be
//portable, but in practice it just doesn't work like that... blegh
#ifdef BUILDWINDOWS
#include <pdcurses.h>
#else
#include <ncurses.h>
#endif

//Deps
#include "log.h"

//Our own crap
#include "mycsv.h"
#include "config.h"
#include "api.h"
#include "screen.h"

#define SMALLINPUTLEN 100


static void error(char * fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   char prepend[] = "ERROR: ";
   char * newfmt = malloc(strlen(prepend) + strlen(fmt) + 1);
   if(newfmt) {
      sprintf(newfmt, "%s%s", prepend, fmt);
      vfprintf(stderr, newfmt, args);
      free(newfmt);
   }
   else {
      vfprintf(stderr, fmt, args);
   }
   exit(1);
}

// Ask user for new login info. Writes login info to token file.
void newlogin(struct LowcapiConfig * config)
{
   char username[SMALLINPUTLEN];
   char password[SMALLINPUTLEN];

   while(1) {
      printw("Username: "); refresh();
      lc_getinput_simple(username, SMALLINPUTLEN);
      printw("\nPassword: "); refresh();
      lc_getpass_simple(password, SMALLINPUTLEN);
      printw("\nLogging in...\n"); refresh();
      char * output = NULL;
      if(lc_consumeresponse(lc_login(username, password, config), &output))
      {
         print_color(LCSCL_OK, "Token received, writing to file\n");
         lc_storetoken(config, output);
         break;
      }
      else 
      {
         print_color(LCSCL_WARN, "Error: %s\n", output ? output : "UNKNOWN");
      }
      free(output); //Apparently it's safe to call free on null
      refresh();
   }
}

long roomsearch(struct HttpRequest * request) 
{
   long roomid = 0;
   char roomname[SMALLINPUTLEN];
   char * output = NULL;
   struct RequestValue * values = NULL;

   // This is the endpoint we'll keep querying
   sprintf(request->endpoint, "small/search");

   while(1)
   {
      // Safety
      free(output);
      lc_freeallvalues(values, NULL);

      // one exit
      if(roomid) return roomid;

      printw("Search for a room or enter an ID: ");
      refresh();
      lc_getinput_simple(roomname, SMALLINPUTLEN);
      printw("\n");
      refresh();

      roomid = atoi(roomname);

      if(roomid)
      {
         values = lc_addvalue(NULL, "id", roomname);
      }
      else
      {
         lc_makesearch(roomname, SMALLINPUTLEN);
         values = lc_addvalue(NULL, "search", roomname);
      }

      if(lc_consumeresponse(lc_getapi(request, values), &output))
      {
         //Need to parse the lines and output each one.
         struct CsvLineCursor cursor = csv_initcursor_f(output);
         while(csv_readline(&cursor))
         {
            if(!lc_verifycontent(&cursor))
               error("Bad format returned from search endpoint");
            if(roomid && atoi(cursor.line->fields[LCKEY_CONTENTID]) == roomid)
               printw("Selecting room %s: '%s'\n", cursor.line->fields[LCKEY_CONTENTID], cursor.line->fields[LCKEY_CONTENTNAME]);
            else
               printw(" %6s - %s\n", cursor.line->fields[LCKEY_CONTENTID], cursor.line->fields[LCKEY_CONTENTNAME]);
         }

         if(cursor.linecount == 0) {
            print_color(LCSCL_WARN, "No results found\n");
            roomid = 0;
         }

         refresh();
      }
      else
      {
         print_color(LCSCL_ERR, "Search error: %s\n", output ? output : "UNKNOWN");
         roomid = 0;
      }
   }
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
   setlocale(LC_ALL, ""); //en_US.UTF-8");
   struct LowcapiConfig config = lc_read_config(argc > 1 ? argv[1] : NULL);

   lc_setup_logging(&config);
   log_info("Program started");
   lc_log_config(&config);

   lc_curlinit();
   lc_setup_screen();

   if(!checkconnection(&config))
      error("Can't continue, cannot reach API at %s", config.api);

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

   print_color(LCSCL_OK, "Logged in as %s (%ld)!\n", me.username, me.userid);
   refresh();

   //This request will be reused for some stuff. it's meant to
   struct HttpRequest request;
   lc_initrequest(&request, "none", &config);
   sprintf(request.token, "%s", token);

   //Also, you can free the token. It's stored in the request
   free(token);

   long roomid = roomsearch(&request);

   log_info("Program end");
   printw("Program end\n");
   refresh();

   getch();

   return 0;
}


