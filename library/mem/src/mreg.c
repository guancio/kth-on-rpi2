
#include <memlib.h>

/*
 * mreg is a two level buddy implementation to save memory.
 * 
 * You can either allocate memory in 1MB multiples, or smaller
 * sizes within a MB (aligned to the usual 2**n order size)
 */

/* make compiler happy */
void *malloc(size_t );
void free(void *);

/* --- debug ---------------------------------------------------------------------*/

void memregion_dump(memregion_t *mem)
{
    if(mem) {
        printf("<@%x: %x-%x,%x,%x>\n", mem,
               mem->start, mem->end, mem->flags, mem->buddy);
        
        if(mem->flags & MEMREGION_FLAGS_IS_BUDDY_PARENT) {
            buddy_dump(mem->buddy);
        }
    } else {
        printf("NULL\n");
    }
}

void mreg_dump(mreg_t *mreg)
{
    memregion_t *tmp;
    
    printf("MREG %x-%x\n", mreg->start, mreg->end);
    
    printf("   FREE 1MB regions:\n");
    LIST_FOREACH_TYPED(& mreg->free_megs, memregion_t, tmp) {
        printf("\t");
        memregion_dump(tmp);
    }
    printf("\n");        
    
    printf("   FREE buddy regions:\n");
    LIST_FOREACH_TYPED(& mreg->free_buddies, memregion_t, tmp) {
        printf("\t");
        memregion_dump(tmp);
        printf("\n");
    }
    printf("\n");        
    
}
/* ---[ memregion ]-------------------------------------------------------------- */

PRIVATE void memregion_free(mreg_t *reg, memregion_t *mem)
{        
    free(mem);
}

PRIVATE memregion_t *memregion_alloc(mreg_t *reg, addr_t start, addr_t end)
{
    memregion_t *ret = malloc( sizeof(memregion_t));
    if(ret) {
        ret->start = start;
        ret->end = end;
        ret->flags = 0;
        ret->buddy = 0;                
    }
    return ret;
}

PRIVATE BOOL memregion_attach_buddy(mreg_t *reg, memregion_t *mem)
{
    buddy_ctxt_t *buddy;
    
    if(mem->flags & (MEMREGION_FLAGS_IS_BUDDY_PARENT | MEMREGION_FLAGS_IS_BUDDY_CHILD)) {
        return FALSE;
    }
    
    buddy = malloc( sizeof(buddy_ctxt_t));
    if(buddy) {        
        if(buddy_init(buddy, mem->start, mem->end - mem->start + 1, reg->page_bits, 0, 0)) {
            mem->buddy = buddy;
            mem->flags |= MEMREGION_FLAGS_IS_BUDDY_PARENT;
            
            list_add( &reg->free_buddies, (list_t *) mem);
            return TRUE;
        }
        free(buddy);
    }
    return FALSE;
}

PRIVATE BOOL memregion_deattach_buddy(mreg_t *reg, memregion_t *mem)
{
    if(mem->flags & MEMREGION_FLAGS_IS_BUDDY_PARENT) {
        buddy_cleanup(mem->buddy);
        free(mem->buddy);
        list_remove( &reg->free_buddies, (list_t *) mem);
        mem->flags &= ~MEMREGION_FLAGS_IS_BUDDY_PARENT;        
        return TRUE;
    }
    return FALSE;
}

/* ----[ level 1 ]------------------------------------------------------------ */
PRIVATE memregion_t *mreg_level1_split(mreg_t *reg, memregion_t *from, size_t size, BOOL keep_first)
{
    /* split extra space and return it one of the two (free the other) */
    memregion_t *second = memregion_alloc(reg, from->start + size, from->end);
    if(!second)
        return 0;    
    
    from->end = from->start + size - 1;
    list_add( &reg->free_megs, (list_t *) (keep_first ? second : from) ); 
    return keep_first ? from : second;
}


PRIVATE memregion_t *mreg_level1_allocate(mreg_t *reg, int count)
{
    size_t size = count << MB_BITS;
    size_t best_size = 0, my_size;
    memregion_t *tmp, *best = 0;
    
    /* search for best fit chunk */
    LIST_FOREACH_TYPED(& reg->free_megs, memregion_t, tmp) {
        my_size = tmp->end - tmp->start + 1;
        if(my_size >= size) {
            if(!best || my_size < best_size) {
                best = tmp;
                best_size = my_size;
            }
        }
    }
    
    /* allocate it! */
    if(best) {
        list_remove( &reg->free_megs, best);                
        
        /* split the rest */        
        if(best_size > size) {
            tmp = mreg_level1_split(reg, best, size, TRUE);            
        }        
    }    
    
    return best;
}

PRIVATE memregion_t *mreg_level1_allocate_at(mreg_t *reg, addr_t adr, int count)
{
    size_t length = count << MB_BITS;
    addr_t end = adr + length - 1;
    memregion_t *tmp, *target = 0;

    /* is this really a 1MB aligned address ? */
    if( adr & (MB-1)) 
        return FALSE;
    
    /* search for the needed chunk */
    LIST_FOREACH_TYPED(& reg->free_megs, memregion_t, tmp) {
        if(tmp->start <= adr && end <= tmp->end) {            
            target = tmp;
            break;
        }
    }
    
    /* found the region we need, take it out */
    if(target) {
        list_remove( &reg->free_megs, target);                
        
        /* free space before */
        if(target->start < adr) {
            target = mreg_level1_split(reg, target, adr - target->start + 0, FALSE);
        }
                
        /* free space after */
        if(target->end > end) {
            target = mreg_level1_split(reg, target, end - target->start + 1, TRUE);
        }        
    }
    
    return target;
}

PRIVATE  memregion_t *mreg_find_starting(mreg_t *reg, addr_t start)
{
    memregion_t *tmp;
    
    LIST_FOREACH_TYPED(&reg->free_megs, memregion_t, tmp) {    
        if(tmp->start == start)
            return tmp;
    }
    return 0;
}

PRIVATE memregion_t *mreg_find_ending(mreg_t *reg, addr_t end)
{
    memregion_t *tmp;
    
    LIST_FOREACH_TYPED(&reg->free_megs, memregion_t, tmp) {    
        if(tmp->end == end)
            return tmp;
    }
    return 0;
}

PRIVATE void mreg_level1_free(mreg_t *reg, memregion_t *mem)
{
    memregion_t *tmp;
        
    list_add( &reg->free_megs, mem);

    /* see if we have it's head */
    tmp = mreg_find_ending(reg, mem->start-1);
    if(tmp) {
        list_remove(& reg->free_megs, mem);        
        tmp->end = mem->end;
        memregion_free(reg, mem);
        mem = tmp;        
    }
    
    /* see if we have it's tail */
    tmp = mreg_find_starting(reg, mem->end+1);
    if(tmp) {
        list_remove(& reg->free_megs, tmp);
        mem->end = tmp->end;
        memregion_free(reg, tmp);
    }
}

/* ----[ level 2 ]------------------------------------------------------------ */

PRIVATE memregion_t *mreg_level2_allocate(mreg_t *reg, size_t size)
{
    memregion_t *tmp, *ret;
    addr_t adr;
    int order;
    
    if(list_is_empty( & reg->free_buddies))
        return 0;
   
    tmp = (memregion_t *) reg->free_buddies.next;
    order = buddy_get_order_from_size(tmp->buddy, size);

    LIST_FOREACH_TYPED(&reg->free_buddies, memregion_t, tmp) {
        if(buddy_alloc(tmp->buddy, order, &adr)) {
            ret = memregion_alloc(reg, adr, adr - 1 + (1UL << (order + reg->page_bits)));
            
            if(ret) {
                ret->buddy = tmp->buddy;
                ret->flags |= MEMREGION_FLAGS_IS_BUDDY_CHILD; 
                return ret;
            }
            buddy_free(tmp->buddy, adr);
        }
    }
  
    return 0; /* FAILED */
}

PRIVATE memregion_t *mreg_level2_allocate_at(mreg_t *reg, addr_t adr, size_t size)
{
    memregion_t *tmp, *ret;
    addr_t adr0;
    int order;
    
    if(list_is_empty( & reg->free_buddies))
        return 0;
    
    tmp = (memregion_t *) reg->free_buddies.next;
    order = buddy_get_order_from_size(tmp->buddy, size);
        
    LIST_FOREACH_TYPED(&reg->free_buddies, memregion_t, tmp) {
        if(buddy_alloc_at(tmp->buddy, adr, order, &adr0)) {   
            ret = memregion_alloc(reg, adr0, adr0 - 1 + (1UL << (order + reg->page_bits)));
            if(ret) {
                ret->buddy = tmp->buddy;
                ret->flags |= MEMREGION_FLAGS_IS_BUDDY_CHILD; 
                return ret;
            }
            buddy_free(tmp->buddy, adr);
        }
    }    
    return 0; /* FAILED */
}

PRIVATE void mreg_level2_free(mreg_t *reg, memregion_t *mem)
{    
    buddy_free(mem->buddy, mem->start);
    free(mem);
}


PRIVATE void mreg_level2_shrink(mreg_t *reg)
{
    memregion_t *tmp;
    int order = MB_BITS - reg->page_bits;
    
    LIST_FOREACH_TYPED(&reg->free_buddies, memregion_t, tmp) {
        if(buddy_contains_order(tmp->buddy, order, FALSE)) {
            
            memregion_t *prev = tmp->prev; 
            memregion_deattach_buddy(reg, tmp); /* this removes us from the loop */
            mreg_level1_free(reg, tmp);            
                        
            tmp = prev; /* this allows safe return to the loop */
        }
    }        
    
}

PRIVATE void mreg_level2_expand(mreg_t *reg)
{
    memregion_t *mem;
    
    mem = mreg_level1_allocate(reg, 1);
    if(mem) {
        if(!memregion_attach_buddy(reg, mem))
            mreg_level1_free(reg, mem);        
    }
}

/* ---[ mreg ]---------------------------------------------------------------- */


mreg_t *mreg_create(int page_bits)
{
    mreg_t *ret;    
    
    /* sanity check */
    if(page_bits > MB_BITS)
        return 0;  
      
    ret = malloc( sizeof(mreg_t));
    if(ret) {
        ret->page_bits = page_bits;
        ret->start = 0;
        ret->end = 0;
        list_init( &ret->free_megs);
        list_init( &ret->free_buddies);    
    }
    
    return ret;
}

BOOL mreg_region_attach(mreg_t *reg, addr_t adr, size_t size)
{
    addr_t adr0, adr1, end0, end1;
    size_t size0, size1, page_size;
    memregion_t *tmp;
    
    page_size = 1UL << reg->page_bits;
    
    /* sanity check */
    if(size < 2 * page_size)
        return FALSE;
            
    /* calculate page and 1MB aligned memory regions */
    adr0 = ALIGN_UP(adr, page_size);
    size0 = ALIGN_DOWN( (size + adr - adr0), page_size);
    end0 = adr0 + size0 - 1;
    
    adr1 = ALIGN_UP(adr0, MB);
    size1 = ALIGN_DOWN( (size0 + adr0 - adr1), MB);
    end1 = adr1 + size1 - 1;
    
    /* TODO: see if this region is already attached */
    
    
    /* add the main region */    
    if(size1 != 0) {
        tmp = memregion_alloc(reg, adr1, end1);
        if(!tmp) 
            return FALSE;    
                
        /* we use mreg_free instead of 
         * list_add(& reg->free_megs, (list_t *) tmp)
         * since it will combine it with adjacent chunks
         */
        mreg_free(reg, tmp);
    }
        
    /* insert the unused pages after 1MB alignment as buddy memory */
    if(adr0 < adr1) {
        tmp = memregion_alloc(reg, adr0, adr1-1);
        if(tmp && !memregion_attach_buddy(reg, tmp)) {
            memregion_free(reg, tmp);
            /* XXX: silent error */
        }            
    }
    
    
    if(end0 != end1 && end0 >= (1 + end1)  ) { /* +1 is wrap around fix */
    // if(end0 > end1) {
        
        tmp = memregion_alloc(reg, end1 + 1, end0);                
        if(tmp && !memregion_attach_buddy(reg, tmp)) {
            memregion_free(reg, tmp);                
            /* XXX: silent error */                
        }            
    }


    /* update size stats */
    if( reg->start == 0 && reg->end == 0) {
        reg->start = adr0;
        reg->end = end0;
    } else {
        if(adr0 < reg->start) reg->start = adr0;
        if(end0 > reg->end) reg->end = end0;
    }
    
     return TRUE;
}

memregion_t *mreg_alloc(mreg_t *reg, size_t size)
{
    
    memregion_t *ret = 0;    
    
    if(size > 0) {        
        size = ALIGN_UP(size, (1UL << reg->page_bits) );
        
        if(size >= MB) {
            size = ALIGN_UP(size, MB);
            ret = mreg_level1_allocate(reg, size >> MB_BITS);
            if(!ret) {
                mreg_level2_shrink(reg);
                ret = mreg_level1_allocate(reg, size >> MB_BITS);                                                
            }            
        } else {
            ret = mreg_level2_allocate(reg, size);
            if(!ret) {
                mreg_level2_expand(reg);
                ret = mreg_level2_allocate(reg, size);
            }
        }
    }        
    
    return ret;
}

memregion_t *mreg_alloc_at(mreg_t *reg, addr_t adr, size_t size)
{
    size_t size0, page_size;
    addr_t adr0, adr1;
    memregion_t *tmp;
    
    
    page_size = 1UL << reg->page_bits;
    
    /* alignment stuff */
    adr0 = ALIGN_DOWN(adr, page_size);
    size0 = ALIGN_UP( (size + adr - adr0), page_size);
    adr1 = ALIGN_DOWN(adr, MB);
    
    /* see if it's within the same MB */
    if( (adr0 >> MB_BITS) != ( (adr0 + size0 -1) >> MB_BITS)) {
        return 0;
    }

    /* try to allocate it from level2 directly */
    tmp = mreg_level2_allocate_at(reg, adr0, size0);
    if(tmp)
        return tmp;
    
    /* see if it is within a free 1MB region */
    tmp = mreg_level1_allocate_at(reg, adr1, 1);
    if(tmp) {        
        /* attach this 1MB region to level2 and allocate it from level2 again */        
        if(memregion_attach_buddy(reg, tmp)) {            
            tmp = mreg_level2_allocate_at(reg, adr0, size0);
        } else {
            /* clean up since we wouldn't attach it */
            mreg_level1_free(reg, tmp);  
            tmp = 0;
        }
    }

    return tmp;
}


void mreg_free(mreg_t *reg, memregion_t *mem)
{    
    if(!mem) return;
    
    if(! (mem->flags & MEMREGION_FLAGS_IS_BUDDY_CHILD) ) {
        //    if(mem->length >= MB) {
        mreg_level1_free(reg, mem);
    } else {
        mreg_level2_free(reg, mem);
    }
}
