#ifndef LC_LINKEDLIST_HEADER
#define LC_LINKEDLIST_HEADER

typedef struct LLNode {
   LLNode * prev;
   LLNode * next;
   void * data;
} LLNode;

LLNode * ll_new();
void ll_delete(LLNode * node);
void 

#endif
