#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#ifdef BUILDWINDOWS
#include <windows.h>
#else
#include <unistd.h>
#include <termios.h>
#endif

#include "lcutils.h"


char * lc_getinput(char * input, size_t maxlen, FILE * stream)
{
   char * result = fgets(input, maxlen, stream);

   if(result) {
      size_t len = strlen(result);
      if(len) {
         result[len - 1] = 0; //Get rid of the newline
      }
   }

   return result;
}

#ifdef BUILDWINDOWS
char * lc_getpass(char * input, size_t maxlen, FILE * stream)
{
   return lc_getinput(input, maxlen, stream);
}
void lc_sleep(long milliseconds)
{
   Sleep(milliseconds);
}
#else
// Taken mostly from https://www.gnu.org/software/libc/manual/html_node/getpass.html
char * lc_getpass(char * input, size_t maxlen, FILE * stream)
{
   struct termios old, new;

   // Turn echoing off and fail if we can’t.
   if (tcgetattr(fileno(stream), &old) != 0) {
      return NULL;
   }

   new = old;
   new.c_lflag &= ~ECHO;
   if (tcsetattr (fileno (stream), TCSAFLUSH, &new) != 0) {
      return NULL;
   }

   // Read the passphrase 
   char * result = lc_getinput(input, maxlen, stream);

   // Restore terminal.
   (void) tcsetattr(fileno(stream), TCSAFLUSH, &old);
   return result;
}
void lc_sleep(long milliseconds)
{
   sleep(milliseconds / 1000);
}
#endif


void error(char * fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   char prepend[] = "ERROR: ";
   char * newfmt = malloc(strlen(prepend) + strlen(fmt) + 1);
   if(newfmt) {
      sprintf(newfmt, "%s%s\n", prepend, fmt);
      vfprintf(stderr, newfmt, args);
      free(newfmt);
   }
   else {
      vfprintf(stderr, fmt, args);
   }
   exit(1);
}

// C is so... mmmm sometimes
void lc_makesearch(char * string, size_t maxlen)
{
   size_t oldlen = strlen(string);
   size_t newlen = oldlen + 2;

   if(newlen > maxlen - 1) {
      newlen = maxlen - 1;
   }

   //Luckily, we know the very end has at least a 0 so...
   for(int i = oldlen; i > 0; i--) {
      string[i] = string[i - 1];
   }

   string[0] = '%';
   string[newlen] = 0;
   string[newlen - 1] = '%';
}

