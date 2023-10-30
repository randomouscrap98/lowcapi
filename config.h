#ifndef LC_CONFIG_GUARD
#define LC_CONFIG_GUARD

#define LC_URLMAXLENGTH 256
#define LC_ERRBUF 512

struct LowcapiConfig
{
   //Yes that's right, the memory is stored in the config object. Sorry!
   char api[LC_URLMAXLENGTH];
   unsigned short initpull;
};

struct LowcapiConfig lc_read_config(const char * filepath);

#endif
