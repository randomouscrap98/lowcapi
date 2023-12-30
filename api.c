#include <curl/curl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "api.h"
#include "config.h"
#include "log.h"

#define LCAPI_URLMAXLENGTH 512

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

// Return a CURL object pointing to the given url with GET, which you need to cleanup
CURL * lc_curlget(char * url)
{
   CURL * curl = curl_easy_init(); 
   if(!curl) {
      error("Couldn't make curl object to %s", url);
   }

   curl_easy_setopt(curl, CURLOPT_URL, url);
   curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");

   return curl;
}

CURL * lc_curlget_api(char * endpoint, struct LowcapiConfig * config)
{
   char url[LCAPI_URLMAXLENGTH];
   snprintf(url, sizeof(url), "%s/%s", config->api, endpoint);
   return lc_curlget(url);
}

// Taken from https://stackoverflow.com/a/2329792. 
// For each chunk reported by curl, increase the response's size and append the
// data to it. 
size_t lc_curl_writecallback(void *contents, size_t size, size_t nmemb, struct HttpResponse * r) {
   size_t conlen = size * nmemb;
   size_t new_len = r->length + conlen; //Length is always just strlen, not + \0
   r->response = realloc(r->response, new_len+1);
   if (!r->response)
      error("Could not (re)allocate response with new data chunk!");
   memcpy(r->response + r->length, contents, conlen);
   r->response[new_len] = '\0';
   r->length = new_len;
   return conlen;
}

struct HttpResponse * lc_curl_setupcallback(CURL * curl, char * url)
{
   struct HttpResponse * response = malloc(sizeof(struct HttpResponse));

   if(!response)
      error("Could not allocate initial response!");

   size_t urllen = sizeof(char) * (strlen(url) + 1);
   response->length = 0;
   response->response = NULL;
   response->status = 0;
   response->url = malloc(urllen);

   if(!response->url)
   {
      lc_freeresponse(response);
      error("Could not allocate initial response (url malloc)!");
   }

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

   //Always copy the response, it might have something valuable (like an error string)
   if(response->response)
   {
      *output = response->response;
      response->response = NULL;
   }

   if(lc_responseok(response))
   {
      result = 1;
   }

   lc_freeresponse(response);

   return result;
}

struct HttpResponse * lc_getany(char * endpoint, struct LowcapiConfig * config, int fail_critical)
{
   //Setup the request, including allocating the initial result object to fill later
   CURL * curl = lc_curlget_api(endpoint, config);
   struct HttpResponse * result = lc_curl_setupcallback(curl, endpoint);

   //Actually make the request, which fills the result object. Also set the status
   CURLcode statusres = curl_easy_perform(curl);
   curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result->status);

   curl_easy_cleanup(curl);
   log_debug("CURL GET: %s", endpoint); //This can expose passwords in the log file!

   if(statusres != CURLE_OK)
   {
      const char * cerr = curl_easy_strerror(statusres);
      LCFAIL(fail_critical, "Couldn't get '%s' endpoint - %s", endpoint, cerr);
   }

   if(!result->response)
      LCFAIL(fail_critical, "Failed to fetch response data for %s endpoint", endpoint);

   return result;
}

struct HttpResponse * lc_login(char * username, char * password, struct LowcapiConfig * config, int fail_critical)
{
   char url[LCAPI_URLMAXLENGTH]; //Is this enough? Usernames can only be 30 or so

   CURL * curlconv = curl_easy_init(); 

   if(!curlconv)
   {
      LCFAIL(fail_critical, "Could not make curl object!");
      return NULL;
   }

   //Have to escape the username and password
   char * usernew = curl_easy_escape(curlconv, username, strlen(username));
   char * passnew = curl_easy_escape(curlconv, password, strlen(password));

   curl_easy_cleanup(curlconv);

   //Only construct a string if they both escaped
   if(usernew && passnew)
   {
      snprintf(url, LCAPI_URLMAXLENGTH, "small/Login?username=%s&password=%s&expireSeconds=%d", 
            usernew, passnew, config->tokenexpireseconds);
   }
   else
   {
      url[0] = 0;
   }

   //Free the unneeded new stuff (we already constructed the url)
   if(usernew) curl_free(usernew);
   if(passnew) curl_free(passnew);

   if(url[0])
   {
      return lc_getany(url, config, fail_critical);
   }
   else
   {
      LCFAIL(fail_critical, "Curl escape failed!");
      return NULL;
   }
}
