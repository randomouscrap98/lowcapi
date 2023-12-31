#include <curl/curl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "api.h"
#include "config.h"
#include "log.h"
#include "mycsv.h"


#define LCFAIL(fc, ...) { if(fc) { error(__VA_ARGS__); } else { log_error(__VA_ARGS__); } }

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

void lc_curlinit()
{
   if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
      error("libcurl initialization failed");
   }
   if (atexit(curl_global_cleanup) != 0) {
      curl_global_cleanup();
      error("couldn't register libcurl cleanup");
   }
   log_debug("Setup libcurl!");
}

// Order doesn't matter (I think). As such, "addvalue" really adds it to the
// front. Much MUCH simpler.
struct RequestValue * lc_addvalue(struct RequestValue * head, char * key, char * value)
{
   struct RequestValue * this = malloc(sizeof(struct RequestValue));
   if(!this) error("Couldn't allocate memory for request value!");
   this->key = key;
   this->value = value;
   this->next = head;

   //Item we created always becomes the new head. This greatly simplifies
   //creating this stuff.
   return this;
}

//Recursively delete the entire value structure
void lc_freeallvalues(struct RequestValue * head, void (*finalize)(struct RequestValue *))
{
   if(head)
   {
      lc_freeallvalues(head->next, finalize);
      if(finalize)
         finalize(head);
      free(head);
   }
}

void lc_initrequest(struct HttpRequest * request, const char * endpoint, struct LowcapiConfig * config)
{
   sprintf(request->endpoint, "%s", endpoint);
   request->config = config;
   request->token[0] = 0;
   request->fail_critical = 0;
}

char * lc_constructurl(struct HttpRequest * request, struct RequestValue * values)
{
   //Create the initial URL, which is the api url plus the request url plus
   //some extra crap. The returned url may be quite large
   size_t basesize = strlen(request->config->api) + strlen(request->endpoint) + 2;
   char * url = malloc(sizeof(char) * basesize);
   if(!url) error("Couldn't allocate url string");
   snprintf(url, basesize, "%s/%s", request->config->api, request->endpoint);

   //Here is where you'd do your value appending
   CURL * curlconv = curl_easy_init(); 
   if(!curlconv) error("Could not make curl object for query param conversion!");

   for(struct RequestValue * this = values; this; this = this->next)
   {
      char * param = curl_easy_escape(curlconv, this->value, strlen(this->value));
      if(!param) error("Couldn't allocate escaped url parameter!");
      // Make a new url that can hold the old one, the key and the value, 
      // the &, the =, and the \0
      size_t oldlen = strlen(url);
      size_t newsize = oldlen + strlen(this->key) + strlen(param) + 3;
      url = realloc(url, sizeof(char) * newsize);
      if(!url) error("Couldn't allocate larger url for new parameter");
      sprintf(url + oldlen, "%c%s=%s", this == values ? '?' : '&', this->key, param);
      curl_free(param);
   }

   curl_easy_cleanup(curlconv);

   return url;
}

// Taken from https://stackoverflow.com/a/2329792. 
// For each chunk reported by curl, increase the response's size and append the
// data to it. 
size_t lc_curl_writecallback(void *contents, size_t size, size_t nmemb, struct HttpResponse * r) 
{
   size_t conlen = size * nmemb;
   size_t new_len = r->length + conlen; //Length is always just strlen, not + \0
   r->response = realloc(r->response, new_len+1);
   if (!r->response) error("Could not (re)allocate response with new data chunk!");
   memcpy(r->response + r->length, contents, conlen);
   r->response[new_len] = '\0';
   r->length = new_len;
   return conlen;
}

struct HttpResponse * lc_curl_setupcallback(CURL * curl, char * url)
{
   struct HttpResponse * response = malloc(sizeof(struct HttpResponse));
   if(!response) error("Could not allocate initial response!");

   size_t urllen = sizeof(char) * (strlen(url) + 1);
   response->length = 0;
   response->response = NULL;
   response->status = 0;
   response->url = malloc(urllen);

   //NOTE: don't need to cleanup stuff since we're using the hard "error" (I think)
   if(!response->url) error("Could not allocate initial response (url malloc)!");

   memcpy(response->url, url, urllen);

   curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, lc_curl_writecallback);
   curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

   return response;
}

void lc_freeresponse(struct HttpResponse * response)
{
   if(!response)
      return;

   if(response->response)
      free(response->response);
   if(response->url)
      free(response->url);

   free(response);
}

int lc_responseok(struct HttpResponse * response)
{
   return response && response->status >= 200 && response->status < 300;
}

//Quick function to log response, check status for specifically OK,
//cleanup the response (free) and return the response body. Make sure
//you free up the returned char*!
int lc_consumeresponse(struct HttpResponse * response, char ** output)
{
   int result = 0;
   log_info("[%ld] - %s", response->status, response->url);

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

struct HttpResponse * lc_getapi(struct HttpRequest * request, struct RequestValue * values)
{
   //Setup the curl request
   struct curl_slist * headers = NULL;
   CURL * curl = curl_easy_init(); 

   if(!curl) error("Couldn't make curl object to %s", request->endpoint);

   //Add the token if it exists
   if(strlen(request->token))
   {
      const char * prepend = "Authorization: Bearer ";
      size_t bearersize = strlen(request->token) + strlen(prepend) + 1;
      char * bearer = malloc(sizeof(char) * bearersize);
      if(!bearer) error("Couldn't allocate memory for token header!");
      snprintf(bearer, bearersize, "%s%s", prepend, request->token);
      headers = curl_slist_append(headers, bearer);
      free(bearer);
   }

   if(headers)
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

   //Setup the url
   char * url = lc_constructurl(request, values);
   curl_easy_setopt(curl, CURLOPT_URL, url);
   curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
   log_trace("CURL GET: %s", url); //This can expose passwords in the log file!
   free(url);

   //Setup the callbacks, this produces the eventual response
   struct HttpResponse * result = lc_curl_setupcallback(curl, request->endpoint);

   //Actually make the request, which fills the result object. Also set the status
   CURLcode statusres = curl_easy_perform(curl);
   curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result->status);

   curl_easy_cleanup(curl);
   if(headers) curl_slist_free_all(headers);

   if(statusres != CURLE_OK)
   {
      const char * cerr = curl_easy_strerror(statusres);
      LCFAIL(request->fail_critical, "Couldn't get '%s' endpoint - %s", request->endpoint, cerr);
   }

   if(!result->response)
      LCFAIL(request->fail_critical, "Failed to fetch response data for %s endpoint", request->endpoint);

   return result;
}

struct HttpResponse * lc_login(char * username, char * password, struct LowcapiConfig * config)
{
   struct HttpRequest request;
   lc_initrequest(&request, "small/Login", config);

   struct RequestValue * values = NULL;
   char expire[16];

   sprintf(expire, "%d", config->tokenexpireseconds);

   values = lc_addvalue(values, "username", username);
   values = lc_addvalue(values, "password", password);
   values = lc_addvalue(values, "expireSeconds", expire);

   struct HttpResponse * result = lc_getapi(&request, values);
   lc_freeallvalues(values, NULL);
   return result;
}


int lc_getme_linefunc(int linenumber, struct CsvLine * line, void * state)
{
   struct MeResponse * me = (struct MeResponse *)state;

   if(line->fieldcount < 2) error("CSV failure: me output missing fields!");

   log_debug("Me result: uid=%s, username=%s", line->fields[0], line->fields[1]);
   me->userid = atoi(line->fields[0]);
   sprintf(me->username, "%s", line->fields[1]);

   return 0;
}

struct MeResponse lc_getme(char * token, struct LowcapiConfig * config)
{
   struct HttpRequest request;
   lc_initrequest(&request, "small/Me", config);
   sprintf(request.token, "%s", token);

   struct HttpResponse * result = lc_getapi(&request, NULL);
   char * text = NULL;

   struct MeResponse me;
   me.userid = 0;
   me.username[0] = 0;

   if(!lc_consumeresponse(result, &text))
   {
      if(text)
         log_error("Error with small/Me: %s", text);
      else
         log_error("Error with small/Me: UNKNOWN");
   }
   else
   {
      if(csv_iteratelines_f(text, lc_getme_linefunc, &me))
         error("Error while parsing me data");
   }

   return me;
}
