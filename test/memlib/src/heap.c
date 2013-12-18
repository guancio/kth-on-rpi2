

#include <memlib.h>
#include <test.h>

#define HEAP_SIZE (1024 * 10)    



/* internal API needed only for testing */
extern heap_marker_t *heap_marker_prev(heap_ctxt_t *heap, heap_marker_t *mark);
extern heap_marker_t *heap_marker_next(heap_ctxt_t *heap, heap_marker_t *mark);
extern void heap_marker_join(heap_ctxt_t *heap, heap_marker_t *m1, heap_marker_t *m2);
extern heap_marker_t *heap_marker_split(heap_ctxt_t *heap, heap_marker_t *mark, int size);


/* ---------------------------------------------------------------- */

heap_marker_t *marker_get_from_address(void *adr)
{
    heap_marker_t *ret =(heap_marker_t *) adr;
    return ret - 1;
}

void *address_get_from_marker(heap_marker_t *m)
{
    return (void *) (m + 1);
}

int marker_get_size(heap_marker_t *m)
{
    return m->size & HEAP_MASK_SIZE;
}

int marker_get_size_prev(heap_marker_t *m)
{
    return m->size_prev;
}

int marker_is_free(heap_marker_t *m)
{
    return (m->size & HEAP_MASK_TAKEN) ? 0 : 1;
}

int marker_is_last(heap_marker_t *m)
{
    return (m->size & HEAP_MASK_LAST) ? 1 : 0;
}

int heap_count_markers(heap_ctxt_t *heap)
{
    int ret;
    heap_marker_t *tmp;
    for(tmp =  heap->memory, ret = 0; tmp; ret++)
        tmp = heap_marker_next(heap, tmp);
    
    return ret;    
}
// --------------------------------------------------------------------


void marker_dump(heap_marker_t *marker)
{
    if(!marker)
        printf("   NULL\n");
    else {
        printf("  %c%c @%08lx  - Size %-8d (Prev %-8d) \n",
               marker_is_free(marker) ? 'F' : '-',
               marker_is_last(marker) ? 'E' : '-',
               address_get_from_marker(marker),
               marker_get_size(marker),
               marker_get_size_prev(marker)
               );
    }
    
}
void dump_heap(heap_ctxt_t *heap) 
{
    heap_marker_t *tmp =  heap->memory;        
    printf("Heap dump:\n");
    while(tmp) {
        marker_dump(tmp);
        tmp = heap_marker_next(heap, tmp);
    }
    printf("\n");
}

// --------------------------------------------------------------------

void test_do(test_context_t *ctx)
{
    
    heap_marker_t *m0, *m1;    
    heap_ctxt_t heap;        
    uint32_t *mem;
    uint32_t *a0, *a1;
    
    
    test_mark(ctx, "heap functions");
    test_group_start(ctx, "Setup and sanity check");
    {
        
        mem = (uint32_t *) malloc(HEAP_SIZE);
        test_not_null(ctx, "Sanity check: allocated heap", mem);                
        heap_init(&heap, HEAP_SIZE, mem);
        
        /* see if the initial block is correctly setup */
        m0 = (heap_marker_t *) mem;        
        test_equal(ctx, "Correct memory size in initial block", 
                   HEAP_SIZE - sizeof(heap_marker_t), 
                   marker_get_size(m0)
                   );
        test_equal(ctx, "Initial block prev size is zero", marker_get_size_prev(m0) , 0);
        test_equal(ctx, "Initial block is free", marker_is_free(m0), 1);
        test_equal(ctx, "Initial block is last", marker_is_last(m0), 1);
        test_equal(ctx, "Only one block initially", heap_count_markers(&heap), 1);
    }
    test_group_end(ctx);
    
    
    test_group_start(ctx, "allocate");
    {
        /* allocate 100 bytes and see what happens */
        a0 = heap_alloc(&heap, 100);
        test_not_null(ctx, "Got 100 bytes", a0);
        test_equal(ctx, "Has two blocks after allocation", heap_count_markers(&heap), 2);
        
        m0 = marker_get_from_address(a0);
        test_larger_than(ctx, "Allocated at least 100 bytes", marker_get_size(m0), 100);        
        test_equal(ctx, "Allocated block is NOT free", marker_is_free(m0), 0);
        
        /* get the other block, we dont know if its before or after */
        m1 = heap_marker_prev(&heap, m0);
        if(m1) {
            /* so prev block is free, and m0 is last */
            test_equal(ctx, "prev allocated block is free", marker_is_free(m1), 1);
            test_equal(ctx, "prev allocated block is NOT last", marker_is_last(m1), 0);
            test_equal(ctx, "Allocated block is last", marker_is_last(m0), 1);

        } else {
            m1 = heap_marker_next(&heap, m0);
            test_not_null(ctx, "Found the remainder block", m1);
            
            /* so m0 is first and ftaken, m1 is last and free */
            test_equal(ctx, "next allocated block is free", marker_is_free(m1), 1);
            test_equal(ctx, "next allocated block is last", marker_is_last(m1), 1);
            
            /* before, so m0 must be the end */
            test_equal(ctx, "Allocated block is NOT last", marker_is_last(m0), 0);
            
        }

    }
    test_group_end(ctx);
    
    
    
    test_group_start(ctx, "allocate another");
    {
        /* allocate another 100 */
        a1 = heap_alloc(&heap, 100);
        test_not_null(ctx, "Got 100 bytes", a1);
        test_equal(ctx, "Has three blocks after another allocation", heap_count_markers(&heap), 3);
        
        m1 = marker_get_from_address(a1);
        test_larger_than(ctx, "Allocated at least 100 bytes", marker_get_size(m1), 100);        
        test_equal(ctx, "Allocated block is NOT free", marker_is_free(m1), 0);
        
        /* verify that a0 and a1 dont overlap */
        if(a0 > a1)
            test_larger_than(ctx, "a0 > a1 + 100 bytes", (uint32_t)a0, (uint32_t)a1 + 100);
        else
            test_larger_than(ctx, "a1 > a0 + 100 bytes", (uint32_t)a1, (uint32_t)a0 + 100);
 

    }
    test_group_end(ctx);    
    
    
 
    test_group_start(ctx, "free");
    {
        heap_free(&heap, a1);
        test_equal(ctx, "Has 2 blocks after first free", heap_count_markers(&heap), 2);
        
        heap_free(&heap, a0);
        test_equal(ctx, "Has 1 block after second free", heap_count_markers(&heap), 1);
        
        /* see if the only marker is correctly setup */
        m0 = (heap_marker_t *) mem;        
        test_equal(ctx, "Correct memory size in only remaining block", 
                   HEAP_SIZE - sizeof(heap_marker_t), 
                   marker_get_size(m0)
                   );
        test_equal(ctx, "Remaining block prev size is zero", marker_get_size_prev(m0) , 0);
        test_equal(ctx, "Remaining block is free", marker_is_free(m0), 1);
        test_equal(ctx, "Remaining block is last", marker_is_last(m0), 1);
        
    }
    test_group_end(ctx);        
}
