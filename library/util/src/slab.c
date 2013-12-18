/*
 * slab object cache implementation
 */

#include <utillib.h>

int slab_init(slab_ctxt_t *ctxt, int item_size, int item_count,
              slab_constructor c, slab_destructor d, void *userdata)
{
    
    ctxt->constructor = c;
    ctxt->destructor = d;
    ctxt->userdata = userdata;
    
    if(! pool_init( & ctxt->pool, item_size, item_count)) {
        return 0;
    }
    
    /* no objects constructed yet */
    ctxt->const_bottom = ctxt->pool.count - 1;
    
    return 1;
}

void slab_cleanup(slab_ctxt_t *ctxt)
{
    /* any data to destruct ?*/
    if(ctxt->destructor) {
        while( ++ctxt->const_bottom < ctxt->pool.count) {
            void *object = pool_get_raw(& ctxt->pool, ctxt->const_bottom);
            ctxt->destructor( object, ctxt->userdata);
        }
    }
    /* now cleanup the pool */
    pool_cleanup(& ctxt->pool);
}

void *slab_alloc(slab_ctxt_t *ctxt)
{
    void *object = pool_alloc( & ctxt->pool);    
        
    if(object) {
        /* see if we have an initialized item at hand */
        if( ctxt->const_bottom == ctxt->pool.current) {
            ctxt->const_bottom--;
            if(ctxt->constructor) 
                ctxt->constructor(object, ctxt->userdata);            
        }
    }
    
    return object;
}

void slab_free(slab_ctxt_t*ctxt, void *x)
{
    pool_free(& ctxt->pool, x);
}


