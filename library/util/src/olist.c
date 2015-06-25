
#include <utillib.h>


/*
typedef struct {
    int curr, max;
    void *list;
} olist_t;
 */

#define SIZE_INITIAL 24
#define SIZE_NEXT(x) ( (x) * 2 + 8)


/* make the list bigger */
static BOOL olist_grow(olist_t *ol)
{
    uint32_t new_size;
    void *new_list;
    void *old_list = ol->list;
    
    /* allocate a new buffer */
    new_size = SIZE_NEXT( ol->size);
    new_list = malloc( new_size * sizeof(void *) );
    if(!new_list) return FALSE;
    
    /* copy the old buffer to the new one */
    memcpy(new_list, old_list, ol->curr * sizeof(void *) );
    
    /* free the old one and set the new one in its place */
    ol->list = new_list;
    ol->size = new_size;
    free(old_list);
    
    return TRUE;
}

/* ------------------------------------------------------------------ */
BOOL olist_init(olist_t *ol)
{
    ol->curr = 0;
    ol->size = SIZE_INITIAL;
    ol->list = malloc( ol->size * sizeof(void *));
    
    return ol->list ? TRUE : FALSE;
}

void olist_cleanup(olist_t *ol)
{
    if(ol->list) {
        free(ol->list);
        ol->list = 0;
    }        
}

/* ------------------------------------------------------------------ */
int olist_size_get(olist_t *ol)
{
    return ol->curr;
}

void *olist_item_get(olist_t *ol, int index)
{
    if(index < 0 || index >= ol->curr) return 0;
    return ol->list[index];
}

void *olist_item_pop(olist_t *ol)
{
    if(ol->curr == 0) return 0;
    
    return ol->list[ -- ol->curr ];
}

BOOL olist_item_push(olist_t *ol, void *item)
{
    if(item == 0) return FALSE;
    
    if(ol->curr >= ol->size - 1) {
        if(! olist_grow(ol))
            return FALSE;
    }
    
    ol->list[ol->curr++] = item;
    return TRUE;
}

void olist_item_remove(olist_t *ol, int index)
{    
    void *last;
    if(index < 0 || index >= ol->curr) return;
    
    /* pop and switch place between that and the removed one (if not the same) */
    last = ol->list[ -- ol->curr ];    
    if(index != ol->curr) 
        ol->list[index] = last;        
}
