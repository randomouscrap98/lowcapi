#include <stdio.h>
#include "toml.h"

#define LC_URLMAXLENGTH 256
const char LC_DEFAULTCONFIG[] = "config.toml";

struct LowcapiConfig
{
   //Yes that's right, the memory is stored in the config object. Sorry!
   char api[LC_URLMAXLENGTH];
   unsigned short initpull;
};

struct LowcapiConfig lc_read_config(const char * filepath)
{
   struct LowcapiConfig result = { 0 };

   if(!filepath)
      filepath = LC_DEFAULTCONFIG;

   return result;
}
