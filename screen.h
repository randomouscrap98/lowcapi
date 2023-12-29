#ifndef LC_SCREEN_GUARD
#define LC_SCREEN_GUARD

#define print_color(color, args...) \
   attron(COLOR_PAIR(color)); \
   printw(args); \
   attroff(COLOR_PAIR(color));


enum {
   LCSCL_OK = 1,
   LCSCL_ERR,
   LCSCL_WARN
};

void lc_setup_screen();

void lc_getany_simple(char * buffer, int maxlength, int password);
// These are both so simple, nobody has to actually define them
#define lc_getpass_simple(b, l) lc_getany_simple(b, l, 1)
#define lc_getinput_simple(b, l) lc_getany_simple(b, l, 0)

#endif
