#include "widget.h"
#include "selectcurses.h"
#include <stdlib.h>

void resize_widget(CWidget * widget, int width, int height)
{
   wresize(widget->window, width, height);
   redraw_widget(widget);
}

void redraw_widget(CWidget * widget)
{
   widget->redraw(widget);
}

void free_widget(CWidget * widget)
{
   free(widget->name);
   free(widget->data);
   delwin(widget->window);
   free(widget);
}

static 
