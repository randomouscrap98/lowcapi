#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include "config.h"
#include "toml.h"
#include "log.h"

#define LCCONF_ERRBUF 512
const char LCCONF_DEFAULTCONFIG[] = "config.toml";

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

   // 1. Read and parse toml file
   fp = fopen(filepath, "r");
   if (!fp) {
      error("Cannot open %s - %s", filepath, strerror(errno));
   }

   char errbuf[LCCONF_ERRBUF];
   toml_table_t* conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
   fclose(fp);

   if (!conf) {
      error("Cannot parse config %s - %s", filepath, errbuf);
   }

   // It's ok if these things aren't found
   loadstring(conf, "api", result.api, sizeof(result.api));
   loadstring(conf, "tokenfile", result.tokenfile, sizeof(result.tokenfile));
   loadstring(conf, "loglevel", result.loglevel, sizeof(result.loglevel));
   loadstring(conf, "logfile", result.logfile, sizeof(result.logfile));

   toml_datum_t initpull = toml_int_in(conf, "initpull");
   if(initpull.ok) { result.initpull = initpull.u.i; }
   toml_datum_t postheight = toml_int_in(conf, "postheight");
   if(postheight.ok) { result.postheight = postheight.u.i; }

   toml_free(conf);
   
   return result;
}

//Perhaps this shouldn't be in this file but whatever. Setup logging
void lc_setup_logging(struct LowcapiConfig * config)
{
   log_set_quiet(true); // We have logging but it should only go to a file

   int level = LOG_INFO;

   if(strcmp(config->loglevel, "trace") == 0)
      level = LOG_TRACE;
   else if(strcmp(config->loglevel, "debug") == 0)
      level = LOG_DEBUG;
   else if(strcmp(config->loglevel, "info") == 0)
      level = LOG_INFO;
   else if(strcmp(config->loglevel, "warn") == 0)
      level = LOG_WARN;
   else if(strcmp(config->loglevel, "error") == 0)
      level = LOG_ERROR;
   else if(strcmp(config->loglevel, "fatal") == 0)
      level = LOG_FATAL;

   log_set_level(level);

   //Setup file logging
   if(strlen(config->logfile)) {
      FILE* fp;
      //NOTE!! Using "w" mode to recreate the file each time! I don't think
      //preserving logs is necessary!!
      fp = fopen(config->logfile, "w");
      if (!fp) {
         error("Cannot open %s for logging", config->logfile);
      }
      //Note: guess we can't close that filepointer. Probably ok?
      log_add_fp(fp, level);
   }
}

//Log the configuration to the logging system
void lc_log_config(struct LowcapiConfig * config)
{
   log_debug("Configuration:\n  api: %s\n  initpull: %d\n  loglevel: %s\n  logfile: %s", 
      config->api, config->initpull, config->loglevel, config->logfile);
}

char * lc_gettoken(struct LowcapiConfig * config)
{
   //Go try to open the file storing the token
   FILE* fp;

   fp = fopen(config->tokenfile, "r");
   if (!fp) {
      log_warn("Could not find token file: %s", config->tokenfile);
      return NULL;
   }

   //Read the token from the file.
   fseek(fp, 0, SEEK_END);
   long fsize = ftell(fp);
   fseek(fp, 0, SEEK_SET);

   char * token = (char *)malloc(fsize + 1);
   if(!token) {
      fclose(fp);
      error("Could not allocate memory for token!");
   }

   size_t bytes_read = fread(token, 1, fsize, fp);
   fclose(fp);
   if(bytes_read != fsize) {
      error("File read failure on token file %s", config->tokenfile);
   }

   token[fsize] = 0;
   return token;
}

void lc_storetoken(struct LowcapiConfig * config, char * token)
{
   //Go try to open the file storing the token
   FILE* fp;

   fp = fopen(config->tokenfile, "w");
   if (!fp) {
      error("Could not open token file for writing: %s", config->tokenfile);
   }

   fprintf(fp, "%s", token);

   fclose(fp);
}
