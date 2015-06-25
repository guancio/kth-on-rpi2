/*
 * object pool implementation
 * 
 * the pool is implemented as a stack, the stack  array and the objects 
 * are allocated as a single memory item.
 * 
 * The object size is aligned to POOL_ALIGN bytes
 * 
 * NOTE: if you change pool implementation, you must also rewrite slab 
 *       implementation, which depends on pool internals.
 */
 

#include <utillib.h>

#define POOL_ALIGN 8
               
int pool_init(pool_ctxt_t *ctxt, int item_size, int item_count)
{
    unsigned char *tmp;
    
    /* check parameter saity */
    if(item_size < LIST_MIN_ITEM_SIZE) item_size = LIST_MIN_ITEM_SIZE;
    if(item_count < 2) item_count = 2;
    
    /* align to POOL_ALIGN */
    item_size = (item_size + POOL_ALIGN -1) & ~(POOL_ALIGN - 1);
    
    /* allocate memory */
    ctxt->count = item_count;
    ctxt->size  = item_size;
    ctxt->current = 0;
    ctxt->list = (void **) malloc(item_size * item_count + sizeof(void *) * item_count);
    if(!ctxt->list) return 0;
    
    /* insert everything into the linked list */
    tmp = (void *) (ctxt->list + sizeof(void *) * item_count);
    while(item_count--) {
        pool_free( ctxt, tmp);
        tmp += item_size;
    }
    
    return 1;        
}

void pool_cleanup(pool_ctxt_t *ctxt)
{
    if(ctxt->list) {
        free(ctxt->list);
        ctxt->list = 0;
    }
}

void *pool_alloc(pool_ctxt_t *ctxt)
{    
    void *ret = 0;
    if(ctxt->current > 0) ret = ctxt->list[--ctxt->current];
    
    return ret;
}

void pool_free(pool_ctxt_t*ctxt, void *x)
{
    if(ctxt->current < ctxt->count) {
        ctxt->list[ctxt->current++] = x;
    }
}

void *pool_get_raw(pool_ctxt_t *ctxt, int index)
{
    char *tmp;
    if(index < 0 || index >= ctxt->count) return 0;
        
    tmp = (void *) (ctxt->list + sizeof(void *) * ctxt->count);
    return tmp + ctxt->size * index;
}
