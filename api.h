#ifndef LC_API_GUARD
#define LC_API_GUARD

#include <curl/curl.h>
#include "config.h"

struct HttpResponse
{
   char * response;
   char * url;
   size_t length;
   long status;
};

//void lc_initresponse(struct HttpResponse * response);
int lc_responseok(struct HttpResponse * response);
void lc_freeresponse(struct HttpResponse * response);
int lc_consumeresponse(struct HttpResponse * response, char ** output);

// Return a CURL object pointing to the given url with GET, which you need to cleanup
void lc_curlinit();
CURL * lc_curlget(char * url);
CURL * lc_curlget_api(char * endpoint, struct LowcapiConfig * config);

//size_t lc_curl_writecallback(void *contents, size_t size, size_t nmemb, char **output);
//void lc_curl_setupcallback(CURL * curl, char ** response);

struct HttpResponse * lc_getany(char * endpoint, struct LowcapiConfig * config, int fail_critical);
struct HttpResponse * lc_login(char * username, char * password, struct LowcapiConfig * config, int fail_critical);

#endif
