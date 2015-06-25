
/*
 * Simply heap allocator, with free and malloc.
 * 
 * due to memory fragmentation problems, use this only as last resort 
 */

#include <memlib.h>


// --------------------------------------------------
PRIVATE heap_marker_t *heap_marker_prev(heap_ctxt_t *heap, heap_marker_t *mark)
{    
    heap_marker_t *ret =
          (mark - 1 - (mark->size_prev / sizeof(heap_marker_t)));    
    return (ret < heap->memory) ? 0 : ret;
}

PRIVATE heap_marker_t *heap_marker_next(heap_ctxt_t *heap, heap_marker_t *mark)
{    
    if( mark->size & HEAP_MASK_LAST) return 0;
    return mark + 1 + (mark->size & HEAP_MASK_SIZE) / sizeof(heap_marker_t);
}

PRIVATE void heap_marker_join(heap_ctxt_t *heap, heap_marker_t *m1, heap_marker_t *m2)
{
    heap_marker_t *next = heap_marker_next(heap, m1);
    
    /* not adjacent */
    if( m2 != next) 
        return;
    
    /* one is free, the other one is not?? */
    if( ((m1->size | m2->size) & HEAP_MASK_TAKEN)) 
        return;
    
    m1->size += (m2->size & HEAP_MASK_SIZE) + sizeof(heap_marker_t);
    m1->size |= m2->size & HEAP_MASK_LAST; /* possibly an end marker */
    
    /* adjust size_prev in next element */
    next = heap_marker_next(heap, m1);
    if(next) 
        next->size_prev = m1->size & HEAP_MASK_SIZE;
    
    /* EXTRA: try to mark the second one invalid */
    m2->size = m2->size_prev = -1;
}

PRIVATE heap_marker_t *heap_marker_split(heap_ctxt_t *heap, heap_marker_t *mark, int size)
{        
    heap_marker_t *tmp = (heap_marker_t *) (size + sizeof(heap_marker_t) + (uint32_t)mark);
    int remain = (mark->size & HEAP_MASK_SIZE) - size;
    
    if(remain > SIZE_MIN) { /* can split, enough space for the next */    
        tmp->size = (remain - sizeof(heap_marker_t)) | (mark->size & HEAP_MASK_FLAGS);
        mark->size = size | (mark->size & HEAP_MASK_TAKEN);
        tmp->size_prev = size;
        
        /* record size change in the next block */
        mark = heap_marker_next(heap, tmp);
        if(mark) {
            mark->size_prev = tmp->size & HEAP_MASK_SIZE;
        }
        return tmp;
    }
    return 0;
}

void heap_init(heap_ctxt_t *heap, uint32_t size, void *memory)
{
    heap_marker_t *first;
    
    size &= ~7; /* size should be multiple of 8 */       
    heap->size = size;
    heap->memory = (heap_marker_t *)memory;
    
    /* insert the large empty block */
    first = (heap_marker_t *) memory;
    first->size  = (size - sizeof(heap_marker_t)) | HEAP_MASK_LAST; /* last */
    first->size_prev = 0;
}

void heap_cleanup(heap_ctxt_t *heap)
{
    /* EMPTY */
}

void *heap_alloc(heap_ctxt_t *heap, int size) 
{
    void *ret = 0;
    heap_marker_t *tmp, *best;
    
    if(size >= 1) {        
        /* size is qword-aligned */
        size = (size + 7) & ~7;
        
        /* find the best match for this size */
        tmp = (heap_marker_t *) heap->memory;
        best = 0;
        
        while(tmp) {        
            uint32_t me = tmp->size;
            if(!(me & HEAP_MASK_TAKEN)) { /* free block */
                if( (me & HEAP_MASK_SIZE) >= size) { /* large enough */
                    if(!best || (me & HEAP_MASK_SIZE) < (best->size & HEAP_MASK_SIZE))
                        best = tmp;                    
                }
            }
            tmp = heap_marker_next(heap, tmp);
        }

        if(best) {            
            /* try to save unused memory */
            heap_marker_split(heap, best, size);
            
            best->size |= HEAP_MASK_TAKEN; /* USED */            
            ret = (void *)(best + 1);
        }
    } 
    return ret;
}

int heap_free(heap_ctxt_t *heap, void *adr)
{    
    heap_marker_t *tmp, *next, *prev;
    /* is marker in this heap ? */    
    tmp = ((heap_marker_t *)adr) - 1;    
    if(tmp < heap->memory || tmp >= (heap->memory + heap->size)) {
#ifdef DEBUG
        printf("Sanity check failure. Is this block is not ours??\n\t%08lx\n", (uint32_t) tmp);
#endif        
        return 0;
    }
    
    prev = heap_marker_prev(heap, tmp);
    next = heap_marker_next(heap, tmp);
    
    /* sanity checks! */
    if( (prev && tmp->size_prev != (prev->size & HEAP_MASK_SIZE) ) ||
        (next && next->size_prev != (tmp->size & HEAP_MASK_SIZE) )) {
#ifdef DEBUG        
        printf("Sanity check failure. Is this block already free??\n\t%08lx\n", (uint32_t) tmp);
#endif        
        return 0;
    }
    
    /* is it really allocated ? */
    if( ! (tmp->size & HEAP_MASK_TAKEN))
        return 0;
    
    tmp->size &= ~ HEAP_MASK_TAKEN; /* now we are free */
    
    if(prev && !(prev->size & HEAP_MASK_TAKEN)) {
        heap_marker_join(heap, prev, tmp);        
        tmp = prev;
    }
    
    if(next && !(next->size & HEAP_MASK_TAKEN))
        heap_marker_join(heap, tmp, next);
    
    return 1;
}

