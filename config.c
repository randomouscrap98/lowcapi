#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "config.h"
#include "toml.h"

#define LCCONF_ERRBUF 512
const char LCCONF_DEFAULTCONFIG[] = "config.toml";

static void error(char* msg, char* msg1)
{
    fprintf(stderr, "ERROR: %s%s\n", msg, msg1?msg1:"");
    exit(1);
}

static int loadstring(toml_table_t * conf, const char * key, char * dest, size_t size)
{
   toml_datum_t value = toml_string_in(conf, key);

   // I'm still a bit rusty on C: apparently strncpy is dumb and bad and rarely
   // does what you want, so people suggested sprintf. I'm using snprintf
   if(value.ok) { 
      snprintf(dest, size, "%s", value.u.s); 
      free(value.u.s); //README says we must free it... 
      return 1;
   }
   else {
      return 0;
   }
}

struct LowcapiConfig lc_read_config(const char * filepath)
{
   struct LowcapiConfig result = { 0 };

   if(!filepath)
      filepath = LCCONF_DEFAULTCONFIG;

   FILE* fp;
   char errbuf[LCCONF_ERRBUF];

   // 1. Read and parse toml file
   fp = fopen(filepath, "r");
   if (!fp) {
      snprintf(errbuf, sizeof(errbuf), "cannot open %s - ", filepath);
      error(errbuf, strerror(errno));
   }

   toml_table_t* conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
   fclose(fp);

   if (!conf) {
      error("cannot parse - ", errbuf);
   }

   // It's ok if these things aren't found
   loadstring(conf, "api", result.api, sizeof(result.api));
   loadstring(conf, "tokenfile", result.tokenfile, sizeof(result.tokenfile));

   toml_datum_t initpull = toml_int_in(conf, "initpull");
   if(initpull.ok) { result.initpull = initpull.u.i; }

   toml_free(conf);
   
   return result;
}


