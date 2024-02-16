#ifndef LC_WIDGET_GUARD
#define LC_WIDGET_GUARD

#include "selectcurses.h"

// So first, is there a "generic" widget? Let's assume not for now
// No hang on... what about sizes and stuff? We can't do.. hm.
// SHOULD we make a widget type? Maaaybe..... 

// resize (also redraws)
// redraw
// events? how? do we give them the means to send and receive events?
// - Maybe? could have a widget builder... then all the implementation
//   specific stuff could be built up using what is essentially 
//   dependency injection.
// - Does every widget need access to the api? I mean if they're 
//   just function pointers.... hmmmm
// - Could network requests be events? You send out events on the
//   event stream, it's picked up, handled, and the result is sent
//   back out? Might make events too... mmm too much processing?
//   How much are they actually using?
// - how do you subscribe to specific events? remember, events 
//   are probably going to be handled as linked lists, one for 
//   each event type. a linked list of event names and then...
//   well no, we have a limited amount of event types because
//   we're going to use an enum. So could just have a static array.
//   Even with 100 event types, that's like 800 bytes.
// - Maybe the creator, since they're doing all the crap anyway, 
//   should hook up the events for you?
// - 

enum Events {
   EVT_NOOP = 1,
   EVT_LOG,
   EVT_END  // Not really meant to be used, only used to determine the number of events
};


typedef struct {
   void (*send_event)(int,void*);
   // When registering a handler, the function needs to take a pointer
   // to the instance data as well as a pointer to the event itself.
   void (*register_handler)(int, void (*handler)(void *, void *));
} EventSystem;


typedef struct CWidget {
   EventSystem * eventsys;
   WINDOW * window;
   void * data;
   // These are part of the "type" of widget, could be moved out but doesn't
   // really need to be (unless the amount of pointers required grows very
   // large and the amount of widgets is extremely large; unlikely ever)
   char * name;
   void (*redraw)(CWidget * widget);
} CWidget;


void resize_widget(CWidget * widget, int width, int height);
void redraw_widget(CWidget * widget);
void free_widget(CWidget * widget);

#endif
