/*
 * mem.c
 *
 *  Created on: Mar 13, 2013
 *      Author: viktordo
 */

/*
 * THE HEAP
 */
#include <memlib.h>
#include <lib.h>

extern addr_t __trusted_heap_start__, __trusted_heap_end__;

static heap_ctxt_t heap;

void free(void *adr){

	heap_free(&heap, adr);
}

void *malloc(uint32_t size){

	return heap_alloc(&heap, size);

}

void *calloc(uint32_t num, uint32_t size){
	/*Unefficient calloc*/

	void *pointer;
	pointer = malloc(num*size);
	memset(pointer,'\0',num*size);
	return pointer;

}

void init_heap(){
    addr_t start = &__trusted_heap_start__;
    addr_t end   = &__trusted_heap_end__;
    heap_init( &heap, end - start - 1, (void *)start);
}
