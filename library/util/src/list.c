/*
 * double-linked list implementation
 */

#include <utillib.h>



void list_init(list_t *list)
{
    list->next = list->prev = list;
}

void list_cleanup(list_t *list)
{
    /* nothing for now */
}

void list_add_between(list_t *first, list_t *second, list_t *item)
{
    second->prev = item;
    item->next = second;
    item->prev = first;
    first->next = item;
}

void list_add(list_t *root, void * item_)
{
    list_t *item = (list_t *) item_;    
   
    if( list_find(root, item)) return; // XXX: ALREADY IN THE LIST!
    list_add_between(root, root->next, item);    
}

void list_remove(list_t *list, void *item_)
{
    list_t *item = (list_t *) item_;
    
    list_t *next = item->next;
    list_t *prev = item->prev;
    
    next->prev = prev;
    prev->next = next;
    
    
    /* this is to avoid problems if we remove it again :( */
    item->next = item->prev = item;
}
    

void *list_pop(list_t *root)
{
    list_t *ret = root->next;
    
    if(ret != root) {
        list_remove(root, ret);
        return ret;
    } else {
        return 0;
    }
}

/* ------------------------------------------------- */
BOOL list_find(list_t *root, void *item_)
{
    list_t *item = (list_t *) item_;
    
    list_t *tmp;
    LIST_FOREACH(root, tmp)
          if(tmp == item) return TRUE;
    return FALSE;
}

BOOL list_is_empty(list_t *root)
{    
    return root->next == root ? TRUE : FALSE;
}

int list_size_get(list_t *root)
{
    int count = 0;    
    list_t *tmp;
    
    LIST_FOREACH(root, tmp) {
        count++;
    }
    
    return count;
}    

