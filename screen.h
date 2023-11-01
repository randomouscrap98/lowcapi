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

#endif
