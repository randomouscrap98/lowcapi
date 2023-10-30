#ifndef LC_CONFIG_GUARD
#define LC_CONFIG_GUARD

#define LCCONF_URLMAXLENGTH 128

struct LowcapiConfig
{
   //Yes that's right, the memory is stored in the config object. Sorry!
   char api[LCCONF_URLMAXLENGTH];
   unsigned short initpull;
   char tokenfile[LCCONF_URLMAXLENGTH];
};

struct LowcapiConfig lc_read_config(const char * filepath);

#endif
