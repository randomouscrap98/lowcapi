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
#define MESSAGELEN 3000
#define DEFAULTPULL 30

#define ERRORSLEEPMS 3000


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

char * get_avatar(CapiValues * capi)
{
   char * avatar = NULL;
   avatar = malloc(LC_USERNAMEMAX + 1);
   if(avatar) {
      HttpResponse * meres = lc_getme(capi);
      if(lc_responseok(meres)) {
         MeResponse me = lc_parseme(meres->response);
         if(strlen(me.avatar) > LC_USERNAMEMAX) {
            error("Avatar field too large!");
         }
         strcpy(avatar, me.avatar);
      }
      else {
         error("Couldn't retrieve avatar, are you not logged in?");
      }
      lc_freeresponse(meres);
   }
   else {
      error("Couldn't allocate memory for avatar!");
   }
   return avatar;
}

void send_action(CapiValues * capi, long room_id, bool lookup_avatar)
{
   char message[MESSAGELEN + 1];
   fprintf(stderr, "Message: ");
   if(!lc_getinput(message, MESSAGELEN, stdin)) {
      error("Couldn't read message?");
   }

   char * avatar = NULL;

   if(lookup_avatar) {
      avatar = get_avatar(capi);
   }

   HttpResponse * messageres = lc_getpost(capi, room_id, message, avatar,
         LC_DEFAULTMARKUP);
   if(!lc_responseok(messageres)) {
      error("Send error: %s", strvalid(messageres->response) ? 
            messageres->response : "UNKNOWN");
   }
   else {
      printf("%s", messageres->response);         
   }
   lc_freeresponse(messageres);
}

typedef struct ListenData {
   char userlist[100 * (LC_USERNAMEMAX + 1)]; //Just some absurd amount, like whatever
   char thisuser[LC_USERNAMEMAX + 1];
   bool parse_output;
   long mid;
   int consolewidth;
} ListenData;

//chatgpt
void print_wrapped_message(const char *text, int width) 
{
   int len = strlen(text);
   int start = 0;
   int end = 0;

   while (start < len) {
      end = start + width;

      if (end >= len) {
         // Print remaining text if it fits in one line
         printf("%.*s\n", len - start, &text[start]);
         break;
      }

      // Find the last space within the line width
      while (end > start && text[end] != ' ')
         end--;

      // If no space found, just print the line width characters
      if (end == start)
         end = start + width;

      // Print the line
      printf("%.*s\n", end - start, &text[start]);

      // Update start for next iteration
      start = end + 1;
   }
}


void print_chatlineparse(struct CsvLine * line, ListenData * ld)
{
   if(!strcmp(line->fields[LCKEY_MSGMODULE], "eventId")) 
   {
      return;
   }
   else if(!strcmp(line->fields[LCKEY_MSGMODULE], "userlist")) 
   {
      // We only really care about the non-zero userlists
      if(strcmp(line->fields[LCKEY_CONTENTID], "0")) 
      {
         // If the userlist is different than what was given, we need to
         // update the userlist and display it
         if (strcmp(line->fields[LCKEY_MSG], ld->userlist)) 
         {
            snprintf(ld->userlist, sizeof(ld->userlist), "%s", line->fields[LCKEY_MSG]);
            printf(CFORE_RESET CFORE_YELLOW "--Userlist: %s\t" CFORE_D_MAGENTA "%s\n",
                  strlen(ld->userlist) ? ld->userlist : "EMPTY", line->fields[LCKEY_MSGDATE]);
         }
      }
   }
   else 
   {
      printf(CFORE_RESET "%s[%s] ", 
         strcmp(line->fields[LCKEY_MSGUSER], ld->thisuser) ? CFORE_B_GREEN : CFORE_B_CYAN,
         line->fields[LCKEY_MSGUSER]);

      if(strchr(line->fields[LCKEY_CONTENTSTATE], 'E')) {
         printf(CFORE_RESET CFORE_RED "%s(edit)", line->fields[LCKEY_CONTENTMSGID]);
      }
      else {
         printf(CFORE_RESET CFORE_D_WHITE "%s", line->fields[LCKEY_CONTENTMSGID]);
      }
      printf("\t" CFORE_RESET CFORE_D_MAGENTA "%s\n", line->fields[LCKEY_MSGDATE]);
      printf(CFORE_RESET);
      print_wrapped_message(line->fields[LCKEY_MSG], ld->consolewidth);
   }

   printf(CFORE_RESET);
}

void handle_listen(HttpResponse * response, ListenData * ld)
{
   ld->consolewidth = lc_console_width();
   if(lc_responseok(response)) {
      // do the thing, we need to scan the csv and get the mid for 
      // future requests. Each line is output differently
      struct CsvLineCursor cursor = csv_initcursor_f(response->response);
      while(csv_readline(&cursor)) //These lines are cleaned up automatically
      {
         if(cursor.error) {
            error("Error while parsing 'chat' data: %d", cursor.error);
         }

         if(cursor.line->fieldcount < LC_CONTENTFIELDS) {
            error("CSV failure: chat output missing fields!");
         }

         long thismid = atol(cursor.line->fields[LCKEY_CONTENTMSGID]);

         if(thismid > ld->mid)
            ld->mid = thismid;

         if(ld->parse_output) {
            print_chatlineparse(cursor.line, ld);
         }
         else {
            cursor.line->start[cursor.line->length - 1] = 0;
            printf("%s\n", cursor.line->start);
         }
      }
   }
   else {
      fprintf(stderr, "Listen error; sleeping for %dms", ERRORSLEEPMS);
      lc_sleep(ERRORSLEEPMS);
   }
   lc_freeresponse(response);
}

// This is actually a loop
void listen_action(CapiValues * capi, long room_id, bool parse_output, long get)
{
   ListenData ld;
   ld.mid = -1;
   ld.userlist[0] = 0;
   ld.parse_output = parse_output;

   char rooms[33];
   sprintf(rooms, "%ld", room_id);

   HttpResponse * meres = lc_getme(capi);
   if(lc_responseok(meres)) {
      MeResponse me = lc_parseme(meres->response);
      if(strlen(me.username) > LC_USERNAMEMAX) {
         error("Username field too large!");
      }
      strcpy(ld.thisuser, me.username);
   }
   else {
      error("Couldn't retrieve username, are you not logged in?");
   }
   lc_freeresponse(meres);

   // First iteration of the loop just pulls some static chat, the rest
   // continuously pulls 
   HttpResponse * response = lc_getchat(capi, ld.mid, -get, rooms);
   handle_listen(response, &ld);

   while(true)
   {
      response = lc_getchat(capi, ld.mid, get, rooms);
      handle_listen(response, &ld);
   }
}


#define require_room(argc) if(argc < 3) { error("You must provide at least a room id!"); }


// ----------------------------
//       Main                  
// ----------------------------

int main(int argc, char * argv[])
{
   if(argc < 2) {
      error("Must provide the action! Usage:\n%s\n%s\n%s\n%s",
            " lowcapi [e]auth",
            " lowcapi [p]search",
            " lowcapi [e]send <roomid>",
            " lowcapi [p]listen <roomid>"
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
      require_room(argc);
      get_base_env(&capi, true);
      send_action(&capi, atol(argv[2]), false);
   }
   else if(!strcmp(argv[1], "esend")) {
      require_room(argc);
      get_base_env(&capi, true);
      send_action(&capi, atol(argv[2]), true);
   }
   else if(!strcmp(argv[1], "listen")) {
      require_room(argc);
      get_base_env(&capi, true);
      listen_action(&capi, atol(argv[2]), false, 
            argc >= 4 ? atol(argv[3]) : DEFAULTPULL);
   }
   else if(!strcmp(argv[1], "plisten")) {
      require_room(argc);
      get_base_env(&capi, true);
      listen_action(&capi, atol(argv[2]), true,
            argc >= 4 ? atol(argv[3]) : DEFAULTPULL);
   }
   else {
      error("Unknown command: %s", argv[1]);
   }


   return 0;
}


