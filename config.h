#ifndef LC_CONFIG_GUARD
#define LC_CONFIG_GUARD

#define LCCONF_URLMAXLENGTH 128

struct LowcapiConfig
{
   //Yes that's right, the memory is stored in the config object. Sorry!
   char api[LCCONF_URLMAXLENGTH];
   int initpull;
   int postheight;
   int tokenexpireseconds;
   char tokenfile[LCCONF_URLMAXLENGTH];
   char logfile[LCCONF_URLMAXLENGTH];
   char loglevel[16];
};

struct LowcapiConfig lc_read_config(const char * filepath);

void lc_setup_logging(struct LowcapiConfig * config);
void lc_log_config(struct LowcapiConfig * config);
char * lc_gettoken(struct LowcapiConfig * config);
void lc_storetoken(struct LowcapiConfig * config, char * token);

#endif
