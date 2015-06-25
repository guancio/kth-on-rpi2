
#include <memlib.h>
#include <test.h>

#define MEM_BASE 1024 * 100
#define MEM_SIZE 51200
#define MEM_PAGE_BITS 12
#define MEM_PAGE_SIZE (1UL << MEM_PAGE_BITS)
#define MEM_PAGE_MASK (MEM_PAGE_SIZE-1)


#define MEM_PAGES_COUNT (MEM_SIZE >> MEM_PAGE_BITS)

extern int rand();


void test_do(test_context_t *test)
{
    buddy_ctxt_t buddy;
    BOOL b;
    uint32_t *mem = malloc( sizeof(uint32_t) * (1 + MEM_PAGES_COUNT));
    uint32_t tmp;
    int i, j;
    
    
    test_mark(test, "buddy memory allocator");
    test_group_start(test, "sanity check");
    {
        b = buddy_init( &buddy, MEM_BASE, MEM_SIZE, MEM_PAGE_BITS, MEM_BASE, MEM_BASE+MEM_SIZE);
        test_equal(test, "Initialized", TRUE, b);                     
    }
    test_group_end(test);    
    
    
    test_group_start(test, "blongs to");
    {
        test_equal(test, "start belongs to", TRUE, 
                   buddy_belongs_to(&buddy, MEM_BASE) );
        
        test_equal(test, "start-1 doesn't belong to", FALSE, 
                   buddy_belongs_to(&buddy, MEM_BASE-1) );
        
        test_equal(test, "end-1 belongs to", TRUE, 
                   buddy_belongs_to(&buddy, MEM_BASE + (MEM_PAGES_COUNT << MEM_PAGE_BITS) -1) );
        
        test_equal(test, "end belongs to", FALSE, 
                   buddy_belongs_to(&buddy, MEM_BASE + (MEM_PAGES_COUNT << MEM_PAGE_BITS)) );
    }
    test_group_end(test);    
    
    
    test_group_start(test, "buddy_get_order_from_size() ");
    {
        test_equal(test, "size = 1 page - 1", 0, 
                   buddy_get_order_from_size(& buddy, MEM_PAGE_SIZE - 1) );
        
        test_equal(test, "size = 1 page + 0", 0, 
                   buddy_get_order_from_size(& buddy, MEM_PAGE_SIZE + 0) );
        
        test_equal(test, "size = 1 page + 1", 1, 
                   buddy_get_order_from_size(& buddy, MEM_PAGE_SIZE + 1) );
        
        
        test_equal(test, "size = 2 page - 1", 1, 
                   buddy_get_order_from_size(& buddy, 2 * MEM_PAGE_SIZE - 1) );
        
        test_equal(test, "size = 2 page + 0", 1, 
                   buddy_get_order_from_size(& buddy, 2 * MEM_PAGE_SIZE + 0) );
        
        test_equal(test, "size = 2 page + 1", 2, 
                   buddy_get_order_from_size(& buddy, 2 * MEM_PAGE_SIZE + 1) );        
        
    }
    test_group_end(test);    
        
    
    test_group_start(test, "allocate memory");
    {
        for(i = 0; i < MEM_PAGES_COUNT; i++) {
            b = buddy_alloc(&buddy, 0, & mem[i]);
            
            if(!b) {
                test_fail(test, "Allocator returned NULL");
            } else {                
                /* see if it is alligned to a page */
                test_equal(test, "allocated memory should be page assigned",
                             0, mem[i] & MEM_PAGE_MASK);
                
                /* see if it is within the memory we have */
                test_larger_or_equal(test, "allocated memory above base",
                                       mem[i], MEM_BASE);
                
                test_less_or_equal(test, "allocated memory below base + size",
                                   mem[i],
                                   MEM_BASE + MEM_PAGE_SIZE * MEM_PAGES_COUNT);
                
                /* hasnt seen this particular address yet: */
                for(j = 0; j < i; j++)
                    test_not_equal(test, "Not returning same address twice",
                                     mem[i], mem[j]);
            }
        }
        
        /* should not have anything more to allocate */
        b = buddy_alloc(&buddy, 0, &tmp);
        test_equal(test, "no more blocks to allocate\n", FALSE, b);
    }
    test_group_end(test);    
    
    
    test_group_start(test, "free memory");
    {
        for(i = 0; i < MEM_PAGES_COUNT; i++) {
            b = buddy_free(&buddy, mem [i]);
            test_equal(test, "freed memory", TRUE, b);
        }
    }
    test_group_end(test);    
    
    test_group_start(test, "allocate at");
    {
        for(j = 0; j < 10; j++) {
            for(i = 0; i < 3; i++) {
                b = buddy_alloc_at(&buddy, MEM_BASE, i, &tmp);
                test_equal(test, "allocate at", TRUE, b);
                test_equal(test, "allocate at address", MEM_BASE, tmp);
                
                b = buddy_free(&buddy, tmp);
                test_equal(test, "allocate at freed", TRUE, b);            
            }        
        }
    }
    test_group_end(test);    
    
    
    test_group_start(test, "higher orders");
    {
        for(i = 0; i < 31; i++) {
            /* allocate and free a page of a higher order */
            b = buddy_alloc(&buddy, i, &tmp);
            
            if( (1UL << i) > MEM_PAGES_COUNT)
                /* we dont have this much memory! */
                test_equal(test, "Should not allocate at this high order", FALSE, b);
            else
                test_equal(test, "Allocated higher order", TRUE, b);
            
            if(b) {
                b = buddy_free(&buddy, tmp);
                test_equal(test, "freed memory", TRUE, b);
            }
            
        }
    }
    test_group_end(test);
    
 
    test_group_start(test, "stress test");
    {
        int r;        
        for( r = 100000; r; --r) {            
            for(i = 0; i < MEM_PAGES_COUNT; i++) {
                b = buddy_alloc(&buddy, 0, & mem[i]);
                test_equal(test, "stress test: alloc ok", TRUE, b);
            }
            
            /* should not have anything more to allocate */
            b = buddy_alloc(&buddy, 0, &tmp);
            test_equal(test, "stress test: no more blocks to allocate\n", FALSE, b);
            
            /* return them in a different order */
            for(j = 0; j < MEM_PAGES_COUNT; j++) {
                int rnd = rand() % MEM_PAGES_COUNT;                
                int tmp = mem[j];                
                mem[j] = mem[rnd];
                mem[rnd] = tmp;
            }
            
            for(i = 0; i < MEM_PAGES_COUNT; i++) {
                b = buddy_free(&buddy, mem [i]);
                test_equal(test, "stress test freed memory", TRUE, b);
            }            
        }
    }
    test_group_end(test);   
    
    test_group_start(test, "first/last free");
    {
        /* first byte in first page and last byte in last page is not free */
        uint32_t first_adr = MEM_BASE + 1;
        uint32_t last_adr  = MEM_BASE + MEM_PAGES_COUNT * MEM_PAGE_SIZE -1;
        
        /* free the old one and allocate a new with first and last byte in use */
        buddy_cleanup(& buddy);                
        b = buddy_init( &buddy, MEM_BASE, MEM_SIZE, MEM_PAGE_BITS, first_adr, last_adr);
        test_equal(test, "allocated first/last buddy", TRUE, b);
        
        
        for(i = 0; i < MEM_PAGES_COUNT; i++) {
            b = buddy_alloc(&buddy, 0, & mem[i]);
            
            if(i < MEM_PAGES_COUNT -2) {
                test_equal(test, "first/last: alloc ok", TRUE, b);
                
                test_larger_than(test, "first/last: not allocating before first", 
                                  mem[i], first_adr);
                
                test_less_than(test, "first/last: not allocating pas last", 
                                  mem[i], last_adr);                
            } else {
                test_equal(test, "first/last: alloc shoud fail", FALSE, b);                
            }                        
        }                
    }
    test_group_end(test);        
    

    test_group_start(test, "border regions");
    {
        addr_t start = 4095UL * MB;
        size_t size;
        
        /* free the old one and allocate a new one the last MB */              
        buddy_cleanup(& buddy);                
        b = buddy_init( &buddy, start, MB , MEM_PAGE_BITS, 0, 0);
        test_equal(test, "border_regions: allocated buddy", TRUE, b);
        
        
        /* normal alloc at the begining */
        for(i = 0; (1UL << (i + MEM_PAGE_BITS)) <= MB; i++) {            
            b = buddy_alloc(&buddy, i, & mem[0]);
            test_equal(test, "border_regions: normal alloc", TRUE, b);
            buddy_free(&buddy, mem[0]);
        }
        
        /* normal alloc_at at the begining */
        for(i = 0; (1UL << (i + MEM_PAGE_BITS)) <= MB; i++) {            
            b = buddy_alloc_at(&buddy, start, i, & mem[0]);
            test_equal(test, "border_regions: normal alloc_at", TRUE, b);
            buddy_free(&buddy, mem[0]);
        }        
        
        /* normal alloc_at at the end */
        for(i = 0; ; i++) {            
            size = 1UL << (i + MEM_PAGE_BITS);
            if(size > MB) break;
            
            b = buddy_alloc_at(&buddy, start - size + MB, i, & mem[0]);
            test_equal(test, "border_regions: alloc_at at the end", TRUE, b);
            buddy_free(&buddy, mem[0]);
        }        
        
        /* normal alloc_at at the end when the first half is taken*/
        for(i = 0; ; i+= 2) {            
            size = 1UL << (i + MEM_PAGE_BITS);
            if(size >= MB) break;
            
            b = buddy_alloc_at(&buddy, start - size * 2 + MB, i, & mem[0]);
            test_equal(test, "border_regions: alloc_at at the end (first)", TRUE, b);
            
            
            b = buddy_alloc_at(&buddy, start - size + MB, i, & mem[1]);
            test_equal(test, "border_regions: alloc_at at the end (Second)", TRUE, b);
            
            buddy_free(&buddy, mem[0]);
            buddy_free(&buddy, mem[1]);
        }                        
    }
    test_group_end(test);        
}
