#include "linkedlist.h"
#include <stdlib.h>


LLNode * ll_new()
{
   LLNode * new = malloc(sizeof(LLNode));

   return new;
}

void ll_delete(LLNode * node)
{
   if(node)
   {
      ll_delete(node->prev); 
      ll_delete(node->next); 
      free(node);
   }
}
