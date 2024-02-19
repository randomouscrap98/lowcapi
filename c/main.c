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

void auth_action(CapiValues * capi, bool use_token)
{
   bool login_required = true;
   char username[SMALLINPUTLEN + 1]; //These are allocated no matter what anyway, so...
   char password[SMALLINPUTLEN + 1];

   // User asked to use the existing token; check it for validity
   if(use_token && strlen(capi->token)) {
      HttpResponse * meres = lc_getme(capi);
      if(lc_responseok(meres)) {
         MeResponse me = lc_parseme(meres->response);
         fprintf(stderr, "Already logged in as %s (%ld)\n", me.username, me.userid);
         login_required = false;
      }
      else {
         fprintf(stderr, "Token invalid or endpoint unreachable\n");
      }
      lc_freeresponse(meres);
   }

   if (login_required) {
      fprintf(stderr, "Username: ");
      if(!lc_getinput(username, SMALLINPUTLEN, stdin)) {
         error("Couldn't read username?");
      }
#ifdef BUILDWINDOWS
      fprintf(stderr, "Password (!!VISIBLE!!): ");
      if(!lc_getpass(password, SMALLINPUTLEN, stdin)) {
         error("Couldn't read password?");
      }
#else
      fprintf(stderr, "Password: ");
      if(!lc_getpass(password, SMALLINPUTLEN, stdin)) {
         error("Couldn't read password?");
      }
      fprintf(stderr, "\n");
#endif
      // Now we can finally check their login
      HttpResponse * loginres = lc_getlogin(capi, username, password);
      if(!lc_responseok(loginres)) {
         error("Login error: %s", strvalid(loginres->response) ? loginres->response : "UNKNOWN");
      }
      else if(!strvalid(loginres->response)){
         error("Login error: nothing returned from endpoint!");
      }
      else if(strlen(loginres->response) > LC_TOKENMAXLENGTH) {
         error("Token too large!");
      }
      else {
         strcpy(capi->token, loginres->response);
      }
      lc_freeresponse(loginres);
   }

   printf("%s\n", capi->token);
}

// Parse each individual search result and output them to stdout. 
void print_searchparse(char * output)
{
   struct CsvLineCursor cursor = csv_initcursor_f(output);
   while(csv_readline(&cursor)) //These lines are cleaned up automatically
   {
      if(cursor.error) {
         error("Error while parsing 'search' data: %d", cursor.error);
      }

      if(cursor.line->fieldcount < LC_CONTENTFIELDS) {
         error("CSV failure: search output missing fields!");
      }

      char private = ' ';

      if(!strchr(cursor.line->fields[LCKEY_CONTENTSTATE], 'R'))
         private = 'P';

      printf("%c%7s - %s\n", private, 
            cursor.line->fields[LCKEY_CONTENTID],
            cursor.line->fields[LCKEY_CONTENTNAME]);
      //print("{}{:>7} - {}".format(private, row[6], row[0]))
      //me.userid = atoi(cursor.line->fields[0]);
      //sprintf(me.username, "%s", cursor.line->fields[1]);
   }
}

void search_action(CapiValues * capi, bool parse_output)
{
   char search[SMALLINPUTLEN + 1];
   fprintf(stderr, "Search: ");
   if(!lc_getinput(search, SMALLINPUTLEN, stdin)) {
      error("Couldn't read search?");
   }

   lc_makesearch(search, SMALLINPUTLEN);

   HttpResponse * searchres = lc_getsearch(capi, search);
   if(!lc_responseok(searchres)) {
      error("Search error: %s", strvalid(searchres->response) ? searchres->response : "UNKNOWN");
   }
   else {
      if(parse_output) {
         print_searchparse(searchres->response);         
      }
      else {
         printf("%s", searchres->response);         
      }
   }
   lc_freeresponse(searchres);
}


// ----------------------------
//       Main                  
// ----------------------------

int main(int argc, char * argv[])
{
   if(argc < 2) {
      error("Must provide the action! Usage:\n%s\n%s\n%s\n%s",
            " lowcapi [e]auth",
            " lowcapi search",
            " lowcapi send <roomid>",
            " lowcapi listen <roomid>"
           );
   }

   lc_curlinit();

   CapiValues capi;

   if(!strcmp(argv[1], "auth")) {
      get_base_env(&capi, false);
      auth_action(&capi, false);
   }
   else if(!strcmp(argv[1], "eauth")) {
      get_base_env(&capi, false);
      auth_action(&capi, true);
   }
   else if(!strcmp(argv[1], "search")) {
      get_base_env(&capi, false);
      search_action(&capi, false);
   }
   else if(!strcmp(argv[1], "psearch")) {
      get_base_env(&capi, false);
      search_action(&capi, true);
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


