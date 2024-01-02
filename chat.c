#include <stdlib.h>
#include <ctype.h>

#include "log.h"
#include "chat.h"
#include "api.h"
#include "config.h"
#include "screen.h"
#include "selectcurses.h"

// This should be configurable later
#define LC_POSTHEIGHT 2
#define LC_INPUTBLOCK 100
#define LC_MAXPOSTLEN 1000

#define LC_REFRESHCHAT() { wrefresh(msgwin); wrefresh(txtwin); wrefresh(spacerwin); }

static void lc_updatespacer(WINDOW * win, char * title)
{
   mvwprintw(win, 0, 0, " - Post: %s -", title ? title : "???");
   wrefresh(win);
}

void lc_runchat(struct HttpRequest * request, long roomid)
{
   int cheight, cwidth;
   getmaxyx(stdscr, cheight, cwidth);
   char post[LC_MAXPOSTLEN + 1];
   int postpos = 0;

   // Create each of the windows
   WINDOW *msgwin, *spacerwin, *txtwin;

   int msgheight = cheight - 1 - LC_POSTHEIGHT;
   int txtstart = msgheight + 1;
   msgwin = newwin(msgheight, cwidth, 0, 0);
   spacerwin = newwin(1, cwidth, msgheight, 0);
   txtwin = newwin(cheight - msgheight, cwidth, txtstart, 0);

   wbkgd(spacerwin, COLOR_PAIR(LCSCL_UI));
   wclear(spacerwin);

   scrollok(msgwin, TRUE);
   wtimeout(txtwin, LC_INPUTBLOCK);

   lc_updatespacer(spacerwin, NULL);

   LC_REFRESHCHAT();

   int i = 0;

   while(1)
   {
      int ch = wgetch(txtwin);

      if(ch != ERR)
      {
         char display = 0;
         int opostpos = postpos;
         // Handle backspace
         if (ch == 127) {
            if(postpos > 0) {
               --postpos;
               display = ' ';
            }
         } else if (isprint(ch) && postpos < LC_MAXPOSTLEN - 1) {
            post[postpos++] = ch;
            display = ch;
         }
         if(display)
         {
            int ty = postpos / cwidth, tx = postpos % cwidth;
            mvwprintw(txtwin, ty, tx, "%c", display); 
            if(opostpos > postpos) wmove(txtwin, ty, tx);
         }
         wrefresh(txtwin);
      }

      // Null-terminate the password string
      //buffer[index] = '\0';

      wprintw(msgwin, "Hey: %d\n", i++);
      wrefresh(msgwin);
   }
}
