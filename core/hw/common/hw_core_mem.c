/*
 * core-mem initialises memory structures before anything else is run
 */

#include <hw.h>

/*
 * THE HEAP
 */
extern addr_t __hyper_heap_start__, __hyper_heap_end__;

static heap_ctxt_t heap;


void* malloc(size_t size)
{
    return heap_alloc(&heap, size);
}

void free(void *ptr) 
{
    if(ptr) 
        heap_free(&heap, ptr);
}


void core_mem_init()
{
    addr_t start = (uint32_t)&__hyper_heap_start__;
    addr_t end   = (uint32_t)&__hyper_heap_end__;
    heap_init( &heap, end - start - 1, (void *)start);    
}
