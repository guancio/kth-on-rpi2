
#ifndef _UTIL_POOL_H_
#define _UTIL_POOL_H_

/* object list: a type of queue without any ordering.
 * works like linked list above but requires less space 
 * and may internally modify the order if items,
 * 
 * NOTE: item must not be NULL
 */
typedef struct {
    int curr, size;
    void **list;
} olist_t;


extern BOOL olist_init(olist_t *);
extern void olist_cleanup(olist_t *cleanup);

extern int   olist_size_get(olist_t *);
extern void *olist_item_get(olist_t *, int index);
extern void *olist_item_pop(olist_t *);
extern BOOL  olist_item_push(olist_t *, void *);
extern void  olist_item_remove(olist_t *, int index);



/* POOL */

typedef struct {
    int count, current;
    int size; /* size of one item */
    void **list;
} pool_ctxt_t;

/* the pool API */
extern int pool_init(pool_ctxt_t *ctxt, int item_size, int item_count);
extern void pool_cleanup(pool_ctxt_t *ctxt);

extern void *pool_alloc(pool_ctxt_t *ctxt);
extern void pool_free(pool_ctxt_t* ctxt, void *);
extern void *pool_get_raw(pool_ctxt_t *ctxt, int index); /* not for normal use! */

/* SLAB */
typedef void (*slab_constructor)(void *obj, void *userdata);
typedef void (*slab_destructor)(void *obj, void *userdata);

typedef struct {
    int const_bottom; /* how low in the stack have we constructed? */    
    pool_ctxt_t pool;    
    
    void *userdata;    
    slab_constructor constructor;
    slab_destructor destructor;
} slab_ctxt_t;

/* the slab API */
extern int slab_init(slab_ctxt_t *ctxt, int item_size, int item_count, 
                     slab_constructor c, slab_destructor d,
                     void *userdata);

extern void slab_cleanup(slab_ctxt_t *ctxt);
extern void *slab_alloc(slab_ctxt_t *ctxt);
extern void slab_free(slab_ctxt_t* ctxt, void *);



#endif /* _UTIL_POOL_ */
