#ifndef LC_LCUTILS_GUARD
#define LC_LCUTILS_GUARD

#ifdef BUILDWINDOWS
/* Windows does not support ANSI escape codes, so define empty macros */
#define CFORE_RESET   ""
#define CFORE_BLACK   ""
#define CFORE_RED     ""
#define CFORE_GREEN   ""
#define CFORE_YELLOW  ""
#define CFORE_BLUE    ""
#define CFORE_MAGENTA ""
#define CFORE_CYAN    ""
#define CFORE_WHITE   ""

#define CFORE_B_BLACK   ""
#define CFORE_B_RED     ""
#define CFORE_B_GREEN   ""
#define CFORE_B_YELLOW  ""
#define CFORE_B_BLUE    ""
#define CFORE_B_MAGENTA ""
#define CFORE_B_CYAN    ""
#define CFORE_B_WHITE   ""

#define CFORE_D_BLACK   ""
#define CFORE_D_RED     ""
#define CFORE_D_GREEN   ""
#define CFORE_D_YELLOW  ""
#define CFORE_D_BLUE    ""
#define CFORE_D_MAGENTA ""
#define CFORE_D_CYAN    ""
#define CFORE_D_WHITE   ""
#else
/* ANSI escape codes for colors */
#define CFORE_RESET   "\x1b[0m"
#define CFORE_BLACK   "\x1b[30m"
#define CFORE_RED     "\x1b[31m"
#define CFORE_GREEN   "\x1b[32m"
#define CFORE_YELLOW  "\x1b[33m"
#define CFORE_BLUE    "\x1b[34m"
#define CFORE_MAGENTA "\x1b[35m"
#define CFORE_CYAN    "\x1b[36m"
#define CFORE_WHITE   "\x1b[37m"

/* Bright versions of ANSI colors */
#define CFORE_B_BLACK   "\x1b[90m"
#define CFORE_B_RED     "\x1b[91m"
#define CFORE_B_GREEN   "\x1b[92m"
#define CFORE_B_YELLOW  "\x1b[93m"
#define CFORE_B_BLUE    "\x1b[94m"
#define CFORE_B_MAGENTA "\x1b[95m"
#define CFORE_B_CYAN    "\x1b[96m"
#define CFORE_B_WHITE   "\x1b[97m"

#define CFORE_D_BLACK   "\x1b[2;30m"
#define CFORE_D_RED     "\x1b[2;31m"
#define CFORE_D_GREEN   "\x1b[2;32m"
#define CFORE_D_YELLOW  "\x1b[2;33m"
#define CFORE_D_BLUE    "\x1b[2;34m"
#define CFORE_D_MAGENTA "\x1b[2;35m"
#define CFORE_D_CYAN    "\x1b[2;36m"
#define CFORE_D_WHITE   "\x1b[2;37m"
#endif

void error(char * fmt, ...);
void lc_makesearch(char * string, size_t maxlen);

char * lc_getpass(char * input, size_t maxlen, FILE * stream);
char * lc_getinput(char * input, size_t maxlen, FILE * stream);

void lc_sleep(long milliseconds);

int lc_console_width();

#define strvalid(s) (s && strlen(s))
#define stringify(s) #s

#endif
