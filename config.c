#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "config.h"
#include "toml.h"

const char LC_DEFAULTCONFIG[] = "config.toml";

static void error(char* msg, char* msg1)
{
    fprintf(stderr, "ERROR: %s%s\n", msg, msg1?msg1:"");
    exit(1);
}

struct LowcapiConfig lc_read_config(const char * filepath)
{
   struct LowcapiConfig result = { 0 };

   if(!filepath)
      filepath = LC_DEFAULTCONFIG;

   FILE* fp;
   char errbuf[LC_ERRBUF];

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
   toml_datum_t api = toml_string_in(conf, "api");
   toml_datum_t initpull = toml_string_in(conf, "initpull");

   // I'm still a bit rusty on C: apparently strncpy is dumb and bad and rarely
   // does what you want, so people suggested sprintf. I'm using snprintf
   if(api.ok) { snprintf(result.api, sizeof(result.api), "%s", api.u.s); }
   if(initpull.ok) { result.initpull = initpull.u.i; }

   return result;
}


