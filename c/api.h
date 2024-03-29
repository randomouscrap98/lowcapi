#ifndef LC_API_GUARD
#define LC_API_GUARD

#include <curl/curl.h>
#include "mycsv.h"

// In our system, all values are added as query parameters. We specially
// separate those to make life "easier" (maybe). As such, the baseline
// endpoint should always be pretty small. If you need to connect to a 
// larger endpoint, IDK... consider making this larger?
#define LC_URLPARTLENGTH 512
#define LC_USERNAMEMAX 50
#define LC_TOKENMAXLENGTH 256

#define LC_TOKENEXPIRE 31536000
#define LC_DEFAULTMARKUP "12y2"
#define LC_COMMENTMARKUP "m"
#define LC_COMMENTAVATAR "a"

#define LCKEY_CONTENTNAME 0
#define LCKEY_MSGUSER 1
#define LCKEY_MSG 2
#define LCKEY_MSGDATE 3
#define LCKEY_MSGMODULE 4
#define LCKEY_CONTENTSTATE 5
#define LCKEY_CONTENTID 6
#define LCKEY_MSGUID 7
#define LCKEY_CONTENTMSGID 8

#define LC_CONTENTFIELDS 9


void lc_curlinit();


typedef struct RequestValue
{
   char * key;
   char * value;
   struct RequestValue * next;
} RequestValue;

RequestValue * lc_addvalue(RequestValue * head, char * key, char * value);
RequestValue * lc_addvalue_l(RequestValue * head, char * key, long value);
void lc_freeallvalues(RequestValue * head, void (*finalize)(RequestValue *));


// Responses can be any size. As such, all of this struct is designed
// for dynamic memory.
typedef struct HttpResponse
{
   char * response;
   char * url;
   size_t length;
   long status;
} HttpResponse;

int lc_responseok(HttpResponse * response);
void lc_freeresponse(HttpResponse * response);
int lc_consumeresponse(HttpResponse * response, char ** output);


// Values passed in from the user which do not change for the duration of the
// run. This api is not meant for longtime use; this is tailor made for the
// needs of the lowcapi interface.
typedef struct CapiValues
{
   char api[LC_URLPARTLENGTH + 1];     //CAPI_URL
   char token[LC_TOKENMAXLENGTH + 1];  //CAPI_TOKEN
   //long room;                          //CAPI_ROOM
} CapiValues;

//This function can handle all simple "get" requests on the api (JUST the api)
HttpResponse * lc_getapi(CapiValues * capi, char * endpoint, RequestValue * values);
HttpResponse * lc_getme(CapiValues * capi);
HttpResponse * lc_getlogin(CapiValues * capi, char * username, char * password);
HttpResponse * lc_getsearch(CapiValues * capi, char * search);
HttpResponse * lc_getpost(CapiValues * capi, long id, char * message, char * avatar,
      char * markup);
HttpResponse * lc_getchat(CapiValues * capi, long mid, long get, char * rooms);

typedef struct MeResponse
{
   long userid;
   char username[LC_USERNAMEMAX + 1];
   char avatar[LC_USERNAMEMAX + 1];
} MeResponse;

MeResponse lc_parseme(char * text);


//int lc_verifycontent(struct CsvLineCursor * cursor);
#endif
