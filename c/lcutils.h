#ifndef LC_LCUTILS_GUARD
#define LC_LCUTILS_GUARD

void error(char * fmt, ...);
void lc_makesearch(char * string, size_t maxlen);

char * lc_getpass(char * input, size_t maxlen, FILE * stream);
char * lc_getinput(char * input, size_t maxlen, FILE * stream);

void lc_sleep(long milliseconds);

#define strvalid(s) (s && strlen(s))
#define stringify(s) #s

#endif
