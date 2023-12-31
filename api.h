#ifndef LC_API_GUARD
#define LC_API_GUARD

#include <curl/curl.h>
#include "config.h"

// In our system, all values are added as query parameters. We specially
// separate those to make life "easier" (maybe). As such, the baseline
// endpoint should always be pretty small. If you need to connect to a 
// larger endpoint, IDK... consider making this larger?
#define LC_ENDPOINTMAXLENGTH 256
#define LC_USERNAMEMAX 50
#define LC_TOKENMAXLENGTH 256

#define LCKEY_CONTENTNAME 0
#define LCKEY_MSGUSER 1
#define LCKEY_MSG 2
#define LCKEY_MSGDATE 3
#define LCKEY_MSGMODULE 4
#define LCKEY_CONTENTSTATE 5
#define LCKEY_CONTENTID 6
#define LCKEY_MSGUID 7
#define LCKEY_CONTENTMSGID 8

void lc_makesearch(char * string, size_t maxlen);

struct RequestValue
{
   char * key;
   char * value;
   struct RequestValue * next;
};

struct RequestValue * lc_addvalue(struct RequestValue * head, char * key, char * value);
void lc_freeallvalues(struct RequestValue * head, void (*finalize)(struct RequestValue *));

// This is a rather large struct, but you can reuse it if you want.
// This is designed so it doesn't need freeing, at the cost of memory.
struct HttpRequest
{
   char endpoint[LC_ENDPOINTMAXLENGTH + 1];
   struct LowcapiConfig * config;
   char token[LC_TOKENMAXLENGTH + 1];
   int fail_critical;
   //NOTE: don't add the dynamic "RequestValues" here, send them
   //separately for the required requests. Some may not even need it
};

void lc_initrequest(struct HttpRequest * request, const char * endpoint, struct LowcapiConfig * config);

// Responses can be any size. As such, all of this struct is designed
// for dynamic memory.
struct HttpResponse
{
   char * response;
   char * url;
   size_t length;
   long status;
};

int lc_responseok(struct HttpResponse * response);
void lc_freeresponse(struct HttpResponse * response);
int lc_consumeresponse(struct HttpResponse * response, char ** output);

struct MeResponse
{
   char username[LC_USERNAMEMAX + 1];
   long userid;
};

void lc_curlinit();

//This first function can handle all simple "get" requests on the api (JUST the api)
struct HttpResponse * lc_getapi(struct HttpRequest * request, struct RequestValue * values);

//These are highly specialized functions that only require a bare minimum but
//assume you DON'T want them to immediately fail on request failure
struct HttpResponse * lc_login(char * username, char * password, struct LowcapiConfig * config);
struct MeResponse lc_getme(char * token, struct LowcapiConfig * config);

#endif
