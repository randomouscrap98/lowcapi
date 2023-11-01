#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "screen.h"

//You're SUPPOSED to be able to use just curses.h on any system and it'll be
//portable, but in practice it just doesn't work like that... blegh
#ifdef BUILDWINDOWS
#include <pdcurses.h>
#else
#include <ncurses.h>
#endif

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

static void lc_end_screen()
{
   endwin();
}

void lc_setup_screen()
{
   initscr();
   start_color();
   cbreak();
   noecho();

   //It's fine if we can't register it... exiting would be silly, it still
   //wouldn't run... maybe? I guess we could call it immediately
   if (atexit(lc_end_screen) != 0) {
      lc_end_screen();
      error("couldn't register libcurl cleanup");
   }

   init_pair(LCSCL_OK, COLOR_GREEN, COLOR_BLACK);
   init_pair(LCSCL_ERR, COLOR_RED, COLOR_BLACK);
   init_pair(LCSCL_WARN, COLOR_YELLOW, COLOR_BLACK);
}

