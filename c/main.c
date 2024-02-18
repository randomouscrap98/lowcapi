#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>

//Our own crap
#include "mycsv.h"
#include "api.h"

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



// ----------------------------
//       Main                  
// ----------------------------

int main(int argc, char * argv[])
{
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

   //Now with the room id, we can setup the basic chatroom. How will it work?
   lc_runchat(&request, roomid);

   log_info("Program end");
   printw("Program end\n");
   refresh();

   getch();

   return 0;
}


