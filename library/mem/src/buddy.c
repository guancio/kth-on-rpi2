
#include <memlib.h>


/* the next field */
#define FLAG_FREE   0x80000000
#define MASK_FLAGS  0xF0000000
#define MASK_NEXT   0x0FFFFFFF
#define GET_NEXT(x) ((x)->next & MASK_NEXT)
#define IS_FREE(x)  ((x)->next & FLAG_FREE)

/* the previous field */
#define SHIFT_ORDER 27
#define MASK_ORDER  0xF8000000
#define MASK_PREV   0x07FFFFFF
#define GET_PREV(x) ((x)->prev & MASK_PREV)
#define GET_ORDER(x) ((x)->prev >> SHIFT_ORDER)


/* ---------------------------------------------------------- */

void buddy_dump(buddy_ctxt_t *buddy)
{
    int i;
    printf("  <BUDDY BASE=%x MAX=%d ORDER=%d>\n", buddy->base, buddy->index_max, buddy->order);
    
    for(i = 0; i < buddy->order; i++) {
        int next, index = buddy->free[i];
        
        if(index != -1) {
            printf("    <ORDER %d>\n", i);             
            for(;;) {
                buddy_block_t *block = buddy->all + index;
                
                if(index < 0 || index >= buddy->index_max) {
                    printf("      <BLOCK *** ERROR index=%d ***>", index);
                } else {
                    addr_t start = buddy->base + (index<< buddy->page_bits);
                    addr_t end   = start + (1UL << (GET_ORDER(block) + buddy->page_bits));

                    printf("      <BLOCK %s A=%x-%x ORDER=%d ME=%d NEXT=%d PREV=%d>\n",
                           IS_FREE(block) ? "free" : "in use",
                           start, end, 
                           GET_ORDER(block),
                           index, GET_NEXT(block), GET_PREV(block)
                           );                    
                }
                next = GET_NEXT(block);
                if(index == next) break;
                index = next;            
            }            
        }
    }
}


/* list operations:
 * 1. buddy->free is empty when index is -1
 * 2. list next/prev is nil if it points to itself
 */


static buddy_block_t *buddy_add_free(buddy_ctxt_t *buddy, int index, int order)
{
    buddy_block_t *block = buddy->all + index;    
    int first = buddy->free[order];
    
    if(first == -1) {  
        block->next = FLAG_FREE | index;
    }  else {
        buddy_block_t *second = buddy->all + first;
        second->prev = (second->prev & MASK_ORDER) | index;
        
        block->next = FLAG_FREE | first;
    }
    
    block->prev = (order << SHIFT_ORDER) | index;    
    buddy->free[order] = index;
    return block;
}

static buddy_block_t *buddy_remove_free(buddy_ctxt_t *buddy, int index, int order)
{
    buddy_block_t *block = buddy->all + index;
    int index_next = GET_NEXT(block);
    int index_prev = GET_PREV(block);
    
    buddy_block_t *next = buddy->all + index_next;
    buddy_block_t *prev = buddy->all + index_prev;
    
    /* adjust next/prev index + free list */
    if(index_prev == index) {
        if(index_next == index) { 
            buddy->free[order] = -1;
        } else {
            buddy->free[order] = index_next;
            next->prev = (next->prev & MASK_ORDER) | index_next;            
        }                
    } else {
        if(index_next == index) { 
            prev->next = (prev->next & MASK_ORDER) | index_prev;
        } else {
            next->prev = (next->prev & MASK_ORDER) | index_prev;
            prev->next = (prev->next & MASK_ORDER) | index_next;            
        }
    }
    return block;
}

/*
 * allocate a subset of a block, return the rest to the free list
 */
static void buddy_allocate_subset(buddy_ctxt_t *buddy, int index1, 
                                  int order1, int index2, int order2)
{
    uint32_t i;
          
    buddy_remove_free(buddy, index1, order1);
    
    /* 
     * ASSERT: GET_ORDER(all) == order1 
     * ASSERT: IS_FREE(all) 
     * ASSERT: index2 + (1UL << order2) <= index1 + (1UL << order1)
     * ASSERT: index2 >= index1
     */
    
    /* mark everything as in use first and with order 0 first */
    for(i = 0; i < (1UL << order1); i++) {
        buddy_block_t *block = buddy->all + index1 + i;
        block->prev = block->next = 0; /* order 0, not free */             
        
        if(index1 + i == index2)
            i += (1UL << order2) - 1;        
    }
        
    /* put back the unused ones, mark our own with correct order */
    for(i = 0; i < (1UL << order1); i++) {
        buddy_block_t *block = buddy->all + index1 + i;
    
        if(index1 + i == index2) {
            block->prev = order2 << SHIFT_ORDER;            
            i += (1UL << order2) - 1;                    
        } else {
            buddy_free(buddy, buddy->base + ((index1 + i) << buddy->page_bits));
        }
    }    
}

/* ---------------------------------------------------- */

int buddy_get_order_from_size(buddy_ctxt_t *buddy, addr_t size)
{
    addr_t ret = buddy->page_bits;
    
    while(size > (1UL << ret)) ret++;
    
    return ret - buddy->page_bits;    
}


BOOL buddy_contains_order(buddy_ctxt_t *buddy, int order, BOOL or_larger)
{
    int i;
    
    if(order < 0 || order >= buddy->order) {
        return 0;
    }    
    
    for(i = order; i < buddy->order; i++) {
        if(buddy->free[i] != -1)
            return TRUE;
        
        if(!or_larger)
            break;
    }
    return FALSE;
}

BOOL buddy_belongs_to(buddy_ctxt_t *buddy, addr_t adr)
{
    uint32_t index = (adr - buddy->base) >> buddy->page_bits;    
    return (index < buddy->index_max) ? TRUE : FALSE;    
}

/* ---------------------------------------------------------- */

BOOL buddy_init(buddy_ctxt_t *buddy, uint32_t base, uint32_t size, int page_bits,
                uint32_t first_free, uint32_t last_free
                )
{
    uint32_t page_size = 1UL << page_bits;
    uint32_t page_mask = page_size - 1;
    int i;
    
    /* base must be a page aligned */
    if( base & page_mask) return FALSE;
       
    buddy->base = base;
    buddy->page_bits = page_bits;    
    size >>= page_bits;
    buddy->index_max = size;
    
    
    /* sanity checks: dont allow too small pages/too little memory */ 
    if(page_bits < 5 || page_bits > 24 || size < 1) return FALSE;
    
    
    /* how many orders will this be? */
    buddy->order = 0;
    while( size > (1UL << buddy->order) ) buddy->order++;
    buddy->order ++; /* allocate an extra one */    
    
    /* allocate the needed memory: */
    buddy->all = (buddy_block_t *) malloc( size * sizeof(buddy_block_t));

    if(!buddy->all) return 0;
    memset(buddy->all, 0, size * sizeof(buddy_block_t));    
    
    /* initialise the free lists and insert everthing as free */
    for(i = 0; i < buddy->order; i++) 
        buddy->free[i] = -1;
    
    /* insert the free pages */
    last_free =  last_free & ~page_mask; /* align down */
    first_free = (first_free + page_mask) & ~page_mask; /* align up */
        
    for(i = 0; i < size; i++) {
        uint32_t adr = base + (i << page_bits);
        if((first_free == 0 || adr >= first_free) && (last_free == 0 || adr < last_free ) ) {
            buddy_free( buddy, adr);
            
        }
    }
        
    return TRUE; /* success! */
}

void buddy_cleanup(buddy_ctxt_t *buddy)
{
    if(buddy->all) {
        free(buddy->all);
        buddy->all = 0;
    }
}

/* -------------------------------------------------------- */

BOOL buddy_alloc(buddy_ctxt_t *buddy, int order, uint32_t *adr)
{
    int i, ret;
    
    for(i = order; i < buddy->order; i++) {
        ret = buddy->free[i];
        if( ret == -1) continue;
        
#if 0        
        /* get this one! */        
        block = buddy_remove_free(buddy, ret, i);
        
        /* mark it as used and give it its correct order */
        block->next &= ~FLAG_FREE;
        block->prev = (block->prev & ~MASK_ORDER) | (order << SHIFT_ORDER);
 
        /* if we have allocated too much, return the reminder */
        while(i-- > order) 
            buddy_add_free(buddy, ret + (1UL << i), i);
#else        
        buddy_allocate_subset(buddy, ret, i, ret, order);
#endif
        
        /* and return the allocated address */                
        *adr = buddy->base + (ret << buddy->page_bits);
        
        return TRUE;
    }    
    
    return FALSE; /* could not allocate */
}

/*
 * allocate at an specific address */
BOOL buddy_alloc_at(buddy_ctxt_t *buddy, uint32_t adr, int order, uint32_t *ret)
{
    int i;
    uint32_t adr2, end2, end1, adr1;
    uint32_t index, next;
    buddy_block_t *block;
    
    adr1 = adr - buddy->base;
    
    /* sanity check: see if it is correctly aligned */
    uint32_t size = 1UL << (order + buddy->page_bits);
    if(adr1 & (size-1)) return FALSE;
    
    adr1 >>= buddy->page_bits;
    end1 = adr1 + (1UL << order) - 1;
        
    /* for all order that may contain this */
    for(i = order; i < buddy->order; i++) {
        index = buddy->free[i];
        if( index == -1) continue;
        
        /* see if we have a block that is a superset or equal to adr1-end2 */
        for(;;) {
            block = buddy->all + index;
            
            adr2 = index;
            end2 = adr2 + (1UL << i);
            
            /* found it! now take it out and return the rest */
            if(adr1 >= adr2 && end1 < end2) {
                buddy_allocate_subset(buddy, index, i, adr1, order);
                *ret = buddy->base + (adr1 << buddy->page_bits); /* = adr */
                return TRUE;
            }
            
            next = GET_NEXT(block);
            if(index == next) break;
            index = next;
        }        
    }    
    return FALSE;
}


BOOL buddy_free(buddy_ctxt_t *buddy, uint32_t adr)
{
    uint32_t page_bits = buddy->page_bits;
//    uint32_t page_size = 1UL << page_bits;
//    uint32_t page_mask = page_size - 1;    
    uint32_t index = (adr - buddy->base) >> page_bits;
    int i, order_max;
        
    if(index < buddy->index_max) {  
        buddy_block_t *block = buddy->all + index;
        buddy_block_t *block_buddy;
        
        
        if( !IS_FREE(block)) {        
            order_max = buddy->order - 1;
            for(i = GET_ORDER(block); i < order_max; i++) {                            
                uint32_t index_buddy = index ^ (1UL << i);                
                if(index_buddy >= buddy->index_max) break;
                                
                block_buddy = buddy->all + index_buddy;                    
                    
                if( !IS_FREE(block_buddy) || GET_ORDER(block_buddy) != i) 
                    break;                                
                
                /* remove the buddy from the free list */
                buddy_remove_free(buddy, index_buddy, i);
                
                /* continue with the start of the new block */
                if(index_buddy < index) {
                    index = index_buddy;
                    block = block_buddy;
                }
            }
            /* mark it as free and insert it into the index */
            buddy_add_free(buddy, index, i);            
            return TRUE; /* successfull free */
        }
    }
    
    return FALSE; /* something bad happened */    
}
 
