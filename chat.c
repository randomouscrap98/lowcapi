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

#define LC_SIMPLEWRAP(win, position) \
{ int cwidth = getmaxx(win); wmove(win, (position) / cwidth, (position) % cwidth); }

int lc_textboxinput(WINDOW * txtwin, int ch, char * buffer, int * position)
{
   if(ch != ERR)
   {
      if (ch == '\n') {
         buffer[*position] = 0;
         *position = 0;
         wclear(txtwin);
         return 1;
      }
      else if (ch == 127 && *position > 0) { //backspace
         // Move backwards first, then use the new position to print 
         *position = *position - 1;
         LC_SIMPLEWRAP(txtwin, *position);
         wprintw(txtwin, " ");
         // Move the cursor back so the user doesn't know we deleted something
         LC_SIMPLEWRAP(txtwin, *position);
      } else if (isprint(ch) && *position < LC_MAXPOSTLEN) {
         // Print at the CURRENT position
         buffer[*position] = ch;
         LC_SIMPLEWRAP(txtwin, *position);
         wprintw(txtwin, "%c", ch);
         // THEN move it forward
         *position = *position + 1;
      }
      wrefresh(txtwin);
   }
   return 0;
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
      if(lc_textboxinput(txtwin, ch, post, &postpos))
      {
         wprintw(msgwin, "Based: %s\n", post);
         wrefresh(msgwin);
      }

      // Null-terminate the password string
      //buffer[index] = '\0';

      //wprintw(msgwin, "Hey: %d\n", i++);
      //wrefresh(msgwin);
   }

   delwin(msgwin);
   delwin(txtwin);
   delwin(spacerwin);
   refresh();
}
