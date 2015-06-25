
#ifndef _MEMLIB_H_
#define _MEMLIB_H_

#include <uclib.h>
#include <utillib.h>

/* some common memory sizes, to make the code more readable */
#define KB_BITS 10
#define MB_BITS 20
#define GB_BITS 30

#define KB (1UL << KB_BITS)
#define MB (1UL << MB_BITS)
#define GB (1UL << GB_BITS)


/* heap */
typedef struct {
    /* size = 2 dword */    
    uint32_t size, size_prev;
} heap_marker_t;

typedef struct {
    uint32_t size, end;
    uint32_t user; /* user data, could fore example be a lock */
    heap_marker_t *memory;
} heap_ctxt_t;

extern void heap_init(heap_ctxt_t *heap, uint32_t size, void *memory);
extern void heap_cleanup(heap_ctxt_t *heap);

extern void *heap_alloc(heap_ctxt_t *heap, int size);
extern int heap_free(heap_ctxt_t *heap, void *adr);

/* these are internal to HEAP and added only for use in low-level test code */
#define HEAP_MASK_TAKEN (1)
#define HEAP_MASK_LAST (2)
#define HEAP_MASK_FLAGS (3)
#define HEAP_MASK_SIZE (~HEAP_MASK_FLAGS)
#define SIZE_MIN (sizeof(heap_marker_t))




/* Buddy */
typedef struct {
    /* a single buddy block. the flags are embedded */
    uint32_t next;
    uint32_t prev;
} buddy_block_t;

typedef struct {
    uint32_t base;
    uint32_t index_max;
    int page_bits;
    int order;
    
    buddy_block_t *all;    
    int free[27]; /* index of start of free list for each order */
} buddy_ctxt_t;

extern BOOL buddy_init(buddy_ctxt_t *buddy, uint32_t base, uint32_t size, int page_bits, 
                       uint32_t first_free, uint32_t last_free);
extern void buddy_cleanup(buddy_ctxt_t *buddy);

extern BOOL buddy_free(buddy_ctxt_t *buddy, uint32_t  adr);
extern BOOL buddy_alloc(buddy_ctxt_t *buddy, int order, uint32_t *adr);
extern BOOL buddy_alloc_at(buddy_ctxt_t *buddy, uint32_t adr, int order, uint32_t *ret);

extern int buddy_get_order_from_size(buddy_ctxt_t *buddy, addr_t size);
extern BOOL buddy_contains_order(buddy_ctxt_t *buddy, int order, BOOL or_larger);
extern BOOL buddy_belongs_to(buddy_ctxt_t *buddy, addr_t adr);

extern void buddy_dump(buddy_ctxt_t *buddy);

/* memreg */

#define MEMREGION_FLAGS_LIB_MASK  0xF0000000
#define MEMREGION_FLAGS_USER_MASK 0x0FFFFFFF

#define MEMREGION_FLAGS_IS_BUDDY_PARENT  0x80000000
#define MEMREGION_FLAGS_IS_BUDDY_CHILD   0x40000000


typedef struct memregion_ {
    /* linked list */
    struct memregion_ *next;
    struct memregion_ *prev;    
    
    /* region variables */
    addr_t start, end;
    uint32_t flags;    
    buddy_ctxt_t *buddy;    
    
    /* user data */
    addr_t udata[2];
    
} memregion_t;


typedef struct mreg_ {
    addr_t start, end;
    int page_bits;  
    list_t free_megs;
    list_t free_buddies;
} mreg_t;


extern mreg_t *mreg_create(int page_bits);
BOOL mreg_region_attach(mreg_t *reg, addr_t adr, size_t size);

// extern mreg_t *mreg_create(addr_t adr, size_t size, int page_bits);
extern memregion_t *mreg_alloc(mreg_t *reg, size_t size);
extern memregion_t *mreg_alloc_at(mreg_t *reg, addr_t adr, size_t size);
extern void mreg_free(mreg_t *reg, memregion_t *mem);

#endif /* _MEMLIB_H_ */
