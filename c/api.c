#include <curl/curl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lcutils.h"
#include "api.h"
#include "mycsv.h"

//#define PRINTURL

void lc_curlinit()
{
   if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
      error("libcurl initialization failed");
   }
   if (atexit(curl_global_cleanup) != 0) {
      curl_global_cleanup();
      error("couldn't register libcurl cleanup");
   }
}

// Order doesn't matter (I think). As such, "addvalue" really adds it to the
// front. Much MUCH simpler.
RequestValue * lc_addvalue(RequestValue * head, char * key, char * value)
{
   RequestValue * this = malloc(sizeof(RequestValue));
   if(!this) {
      error("Couldn't allocate memory for request value!");
   }
   this->key = malloc(strlen(key) + 1);
   if(!this->key) { error("Couldn't allocate buffer for param key!"); }
   strcpy(this->key, key);
   this->value = malloc(strlen(value) + 1);
   if(!this->value) { error("Couldn't allocate buffer for param value!"); }
   strcpy(this->value, value);
   this->next = head;

   //Item we created always becomes the new head. This greatly simplifies
   //creating this stuff.
   return this;
}

RequestValue * lc_addvalue_l(RequestValue * head, char * key, long value)
{
   char buffer[33];
   sprintf(buffer, "%ld", value);
   return lc_addvalue(head, key, buffer);
}

//Recursively delete the entire value structure
void lc_freeallvalues(RequestValue * head, void (*finalize)(RequestValue *))
{
   if(head)
   {
      lc_freeallvalues(head->next, finalize);
      if(finalize)
         finalize(head);
      free(head->key);
      free(head->value);
      free(head);
   }
}

char * lc_constructurl(CapiValues * capi, char * endpoint, RequestValue * values)
{
   //Create the initial URL, which is the api url plus the request url plus
   //some extra crap. The returned url may be quite large
   size_t basesize = strlen(capi->api) + strlen(endpoint) + 2;
   char * url = malloc(sizeof(char) * basesize);
   if(!url) {
      error("Couldn't allocate url string");
   }
   snprintf(url, basesize, "%s/%s", capi->api, endpoint);

   //Here is where you'd do your value appending
   CURL * curlconv = curl_easy_init(); 
   if(!curlconv) {
      error("Could not make curl object for query param conversion!");
   }

   for(RequestValue * this = values; this; this = this->next)
   {
      char * param = curl_easy_escape(curlconv, this->value, strlen(this->value));
      if(!param) {
         error("Couldn't allocate escaped url parameter!");
      }
      // Make a new url that can hold the old one, the key and the value, 
      // the &, the =, and the \0
      size_t oldlen = strlen(url);
      size_t newsize = oldlen + strlen(this->key) + strlen(param) + 3;
      url = realloc(url, sizeof(char) * newsize);
      if(!url) {
         error("Couldn't allocate larger url for new parameter");
      }
      sprintf(url + oldlen, "%c%s=%s", this == values ? '?' : '&', this->key, param);
      curl_free(param);
   }

   curl_easy_cleanup(curlconv);

   return url;
}

// Taken from https://stackoverflow.com/a/2329792. 
// For each chunk reported by curl, increase the response's size and append the
// data to it. 
size_t lc_curl_writecallback(void *contents, size_t size, size_t nmemb, HttpResponse * r) 
{
   size_t conlen = size * nmemb;
   size_t new_len = r->length + conlen; //Length is always just strlen, not + \0
   r->response = realloc(r->response, new_len+1);
   if (!r->response) {
      error("Could not (re)allocate response with new data chunk!");
   }
   memcpy(r->response + r->length, contents, conlen);
   r->response[new_len] = '\0';
   r->length = new_len;
   return conlen;
}

HttpResponse * lc_curl_setupcallback(CURL * curl, char * url)
{
   HttpResponse * response = malloc(sizeof(HttpResponse));
   if(!response) {
      error("Could not allocate initial response!");
   }

   size_t urllen = sizeof(char) * (strlen(url) + 1);
   response->length = 0;
   response->response = calloc(1, 1);
   response->status = 0;
   response->url = malloc(urllen);

   //NOTE: don't need to cleanup stuff since we're using the hard "error" (I think)
   if(!response->url) {
      error("Could not allocate initial response (url malloc)!");
   }

   memcpy(response->url, url, urllen);

   curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, lc_curl_writecallback);
   curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

   return response;
}

void lc_freeresponse(HttpResponse * response)
{
   if(!response)
      return;

   if(response->response)
      free(response->response);
   if(response->url)
      free(response->url);

   free(response);
}

int lc_responseok(HttpResponse * response)
{
   return response && response->status >= 200 && response->status < 300;
}

//Quick function to log response, check status for specifically OK,
//cleanup the response (free) and return the response body. Make sure
//you free up the returned char*!
int lc_consumeresponse(HttpResponse * response, char ** output)
{
   int result = 0;
   //log_info("[%ld] - %s", response->status, response->url);

   //Always move the response, it might have something valuable (like an error string)
   if(response->response)
   {
      *output = response->response;
      response->response = NULL;
   }

   if(lc_responseok(response))
      result = 1;

   lc_freeresponse(response);

   return result;
}

HttpResponse * lc_getapi(CapiValues * capi, char * endpoint, RequestValue * values)
{
   //Setup the curl request
   struct curl_slist * headers = NULL;
   CURL * curl = curl_easy_init(); 

   if(!curl) {
      error("Couldn't make curl object to %s", endpoint);
   }

   //Add the token if it exists
   if(strlen(capi->token))
   {
      const char * prepend = "Authorization: Bearer ";
      size_t bearersize = strlen(capi->token) + strlen(prepend) + 1;
      char * bearer = malloc(sizeof(char) * bearersize);
      if(!bearer) {
         error("Couldn't allocate memory for token header!");
      }
      snprintf(bearer, bearersize, "%s%s", prepend, capi->token);
      headers = curl_slist_append(headers, bearer);
      free(bearer);
   }

   if(headers) {
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
   }

   //Setup the url
   char * url = lc_constructurl(capi, endpoint, values);
   curl_easy_setopt(curl, CURLOPT_URL, url);
   curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
   free(url);

   //To try to prevent weird issues after going to sleep
   curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
   curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 30L);
   curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 30L);

   //Setup the callbacks, this produces the eventual response
   HttpResponse * result = lc_curl_setupcallback(curl, endpoint);

   //Actually make the request, which fills the result object. Also set the status
   CURLcode statusres = curl_easy_perform(curl);
   curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result->status);

   curl_easy_cleanup(curl);
   if(headers) {
      curl_slist_free_all(headers);
   }

   if(statusres != CURLE_OK)
   {
      const char * cerr = curl_easy_strerror(statusres);
      error("Couldn't get '%s' endpoint - %s", endpoint, cerr);
   }

   if(!result->response) {
      error("Program error: no response set!!");
   }

   return result;
}

HttpResponse * lc_getme(CapiValues * capi)
{
   return lc_getapi(capi, "small/Me", NULL);
}

HttpResponse * lc_getlogin(CapiValues * capi, char * username, char * password)
{
   RequestValue * values = NULL;
   char expire[16];

   sprintf(expire, "%d", LC_TOKENEXPIRE); //TODO: make this configurable

   values = lc_addvalue(values, "username", username);
   values = lc_addvalue(values, "password", password);
   values = lc_addvalue(values, "expireSeconds", expire);

   HttpResponse * result = lc_getapi(capi, "small/login", values);
   lc_freeallvalues(values, NULL);
   return result;
}

MeResponse lc_parseme(char * text)
{
   if(!(text && strlen(text))) {
      error("Invalid argument: empty!");
   }

   MeResponse me;
   me.userid = 0;
   me.username[0] = 0;

   struct CsvLineCursor cursor = csv_initcursor_f(text);
   while(csv_readline(&cursor))
   {
      if(cursor.error) {
         error("Error while parsing 'me' data: %d", cursor.error);
      }

      if(cursor.line->fieldcount < 2) {
         error("CSV failure: me output missing fields!");
      }

      me.userid = atoi(cursor.line->fields[0]);
      snprintf(me.username, LC_USERNAMEMAX, "%s", cursor.line->fields[1]);
      snprintf(me.avatar, LC_USERNAMEMAX, "%s", cursor.line->fields[2]);
   }

   return me;
}

HttpResponse * lc_getsearch(CapiValues * capi, char * search)
{
   RequestValue * values = NULL;
   values = lc_addvalue(values, "search", search);
   HttpResponse * result = lc_getapi(capi, "small/search", values);
   lc_freeallvalues(values, NULL);
   return result;
}

HttpResponse * lc_getpost(CapiValues * capi, long id, char * message,
      char * avatar, char * markup)
{
   RequestValue * values = NULL;
   values = lc_addvalue(values, "message", message);
   if(avatar) {
      values = lc_addvalue(values, "values["LC_COMMENTAVATAR"]", avatar);
   }
   if(markup) {
      values = lc_addvalue(values, "values["LC_COMMENTMARKUP"]", markup);
   }
   char url[LC_URLPARTLENGTH + 1];
   snprintf(url, LC_URLPARTLENGTH, "small/post/%ld", id);
   HttpResponse * result = lc_getapi(capi, url, values);
   lc_freeallvalues(values, NULL);
   return result;
}

HttpResponse * lc_getchat(CapiValues * capi, long mid, long get, char * rooms)
{
   RequestValue * values = NULL;
   values = lc_addvalue_l(values, "mid", mid);
   values = lc_addvalue_l(values, "get", get);
   values = lc_addvalue(values, "rooms", rooms);
   HttpResponse * result = lc_getapi(capi, "small/chat", values);
   lc_freeallvalues(values, NULL);
   return result;
}

