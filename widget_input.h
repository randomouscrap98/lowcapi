#ifndef __HEADER_WIDGET_INPUT__
#define __HEADER_WIDGET_INPUT__

#include "selectcurses.h"

// - Inputs assume they have access to the whole window
// - 

typedef struct
{
   WINDOW * window;  // Everything works off this 

} WgMultilineInput;

// Another thought:
// - Full print the entire input for every change. Apparently ncurses is 
//   efficient and only updates the changes anyway
// - Use the same method for printing to determine what a specific 
//   cursor location actually means
// - No horizontal scrolling, but allow vertical? Or both? Maybe both...
//   Yes, if wordwrap is on, scrolling is vertical. Otherwise, scrolling
// - OOOR, the number of lines determines the scroll. Don't need to 
//   support EVERY use case!!! deal with it!
// - horizontal scroll if 1 line, vertical scroll with word wrap if multiple
// - Must still know startx,

// A single line input that supports arrow movement and auto scrolls
// if it's too long. 
//typedef struct 
//{
//   char * buffer;    // Buffer to hold input
//      int bufsize;   // REAL size of the buffer, actual input is -1
//      int inputlen;  // Length of current input
//      int startx;    // Beginning of input
//      int starty;    
//      int width;     // Size of input. Height 1 = horizontal scroll,
//      int height;    // more = vertical scroll
//     char hide;      // If set, all characters typed will be this
//     char base;      // If set, all characters untyped will be this
//      int cursorx;   // Current cursor x (can't rely on ncurses cursor)
//      int cursory;
//
//   // Old thoughts
//// Notes: 
//// - SCROLLING, 1 LINE INPUT! SO MUCH SIMPLER
//// - IF YOU NEED TO PREVIEW, JUST MAKE A PREVIEW
//
//// No... hang on. typing code. inserting actual newlines. this gets
//// way too hard with a single input line. ugh.
//// you may have to just accept that doing this will require a lot of memory 
//// and be very very complicated. ugly
//
//   //// ------------------
//   //char * buffer;    // Buffer which holds the input as it is typed
//   //   int bufsize;   // REAL size of the buffer; actual input is thus -1
//   //   int inputlen;  // Length of current input
//   //   int position;  // Cursor position within buffer
//   //   int offset;    // Visual offset of text (for overflow)
//   //   int width;     // Width of the input
//   //   int newlines;  // Enter newlines
//   //  char hide;      // If set, all characters typed will be this
//   //  char base;      // If set, all characters untyped will be this
//   //   int startx;    // Beginning of input
//      
//} WgInputTracking;
//
//// Update input tracking using the given character. 
////   0 = Still processing input
////   1 = Done processing 
////  -1 = Exited processing pre-emptively
//int wg_trackinput(WgInputTracking * tracker, int ch);
//
//// Draw the input onto the given window. Sets cursor position.
//// Does NOT refresh, you must still do this after
//void wg_drawtrackedinput(WgInputTracking * tracker, WINDOW * win);
//
#endif
