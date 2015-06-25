


#include <memlib.h>
#include <test.h>

extern int rand();

/* internal API */
extern void mreg_level2_shrink(mreg_t *reg);


/* ------------------------------------------------------ */

#define COUNT 64

#define PAGE_BITS 12
#define PAGE_SIZE (1UL << PAGE_BITS)

#define MSIZE(mem)  ((mem)->end - (mem)->start + 1)


size_t level1_size_now, level1_size_org;
int level1_count_now, level1_count_org;
int level2_count_now, level2_count_org;

/* helper functions */
void get_level1_stats(mreg_t *reg, int *count_, size_t *size_)
{
    memregion_t *tmp;    
    int count = 0;
    size_t size = 0;    
    
    LIST_FOREACH_TYPED(&reg->free_megs, memregion_t, tmp) {    
        count++;
        size += MSIZE(tmp);
    }
    
    *count_ = count;
    *size_ = size;
}

void get_level2_stats(mreg_t *reg, int *count_)
{
    memregion_t *tmp;    
    int count = 0;
    
    LIST_FOREACH_TYPED(&reg->free_buddies, memregion_t, tmp) {
        count++;
    }
    
    *count_ = count;
}

void check_restored(test_context_t *test, mreg_t *reg)
{    
    mreg_level2_shrink(reg); /* needed to move free level2 entries into level1 */
    
    get_level1_stats(reg, &level1_count_now, &level1_size_now);
    get_level2_stats(reg, &level2_count_now);
    
    test_equal(test, "restored: level1 count", level1_count_now, level1_count_org);
    test_equal(test, "restored: level1 size", level1_size_now, level1_size_org);
    test_equal(test, "restored: level2 count", level2_count_now, level2_count_org);    
}
/* ------------------------------------------------------ */

void test_do(test_context_t *test)
{
    int i, j;
    memregion_t *buffer[COUNT];
    mreg_t *reg;
    BOOL succ;
    
    test_mark(test, "mreg functions");
    test_group_start(test, "Setup and sanity check");
    {
        reg = mreg_create(PAGE_BITS);
        test_not_null(test, "Sanity check: created mreg", reg);                
        
        buffer[0] = mreg_alloc(reg, PAGE_BITS);
        test_null(test, "Sanity check: alloc from empty mreg", buffer[0]);
        
        buffer[1] = mreg_alloc_at(reg, 0, PAGE_BITS);
        test_null(test, "Sanity check: alloc from empty mreg", buffer[1]);
        
        
        succ = mreg_region_attach(reg, MB - KB * 8, MB * 3 + KB * 16);
        test_equal(test, "Sanity check: added required region mreg", TRUE, succ);
        
        get_level1_stats(reg, &level1_count_org, &level1_size_org);
        get_level2_stats(reg, &level2_count_org);
        
        test_equal(test, "Sanity check: level1 count at start", 1, level1_count_org);
        test_equal(test, "Sanity check: level1 size at start", MB * 3, level1_size_org);
        test_equal(test, "Sanity check: level2 count at start", 2, level2_count_org);
    }
    test_group_end(test);
    
    test_group_start(test, "alloc (large)");
    {
        
        buffer[0] = mreg_alloc(reg, MB);
        get_level1_stats(reg, &level1_count_now, &level1_size_now);        
                
        test_not_null(test, "alloc large: got 1", buffer[0]);   
        test_equal(test, "alloc large: got 1, size 1MB", MB, MSIZE(buffer[0]));
        test_equal(test, "alloc large: got 1, total -1MB", 
                   level1_size_now, level1_size_org - MB);
                
        buffer[1] = mreg_alloc(reg, MB);
        get_level1_stats(reg, &level1_count_now, &level1_size_now);        
        
        test_not_null(test, "alloc large: got 2", buffer[1]);   
        test_equal(test, "alloc large: got 2, size 1MB", MB, MSIZE(buffer[1]));
        test_equal(test, "alloc large: got 2, total -2MB", 
                   level1_size_now, level1_size_org - 2 * MB);
        
        
        buffer[2] = mreg_alloc(reg, MB);
        get_level1_stats(reg, &level1_count_now, &level1_size_now);        
        
        test_not_null(test, "alloc large: got 3", buffer[2]);   
        test_equal(test, "alloc large: got 3, size 1MB", MB, MSIZE(buffer[2]));
        test_equal(test, "alloc large: got 3, total -3MB", 
                   level1_size_now, level1_size_org - 3 * MB);
        
        
        buffer[3] = mreg_alloc(reg, MB);
        get_level1_stats(reg, &level1_count_now, &level1_size_now);        
        
        test_null(test, "alloc large: didn't get 4", buffer[3]);   
        test_equal(test, "alloc large: didn't get 3, total still -3MB", 
                   level1_size_now, level1_size_org - 3 * MB);                
    }
    test_group_end(test);
    
    
    test_group_start(test, "free (large)");
    {        
        mreg_free(reg, buffer[0]);
        mreg_free(reg, buffer[1]);
        mreg_free(reg, buffer[2]);
        mreg_free(reg, buffer[3]);
        
        check_restored(test, reg);
    }
    test_group_end(test);
    
    
    test_group_start(test, "alloc (small)");
    {                
        /* gets 8 KB from first l2 */
        buffer[0] = mreg_alloc(reg, KB * 8);        
        get_level1_stats(reg, &level1_count_now, &level1_size_now);                
        get_level2_stats(reg, &level2_count_now);
        
        test_not_null(test, "alloc small: got 1", buffer[0]);   
        test_equal(test, "alloc small: got 1, size 8KB", 8 * KB, MSIZE(buffer[0]));
        
        test_equal(test, "alloc small: got 1, level 1 size unchanged", 
                   level1_size_now, level1_size_org);
        
        test_equal(test, "alloc small: got 1, level 2 count unchanged", 
                   level2_count_now, level2_count_org);        
        
        /* gets 8 KB from second l2 */
        buffer[1] = mreg_alloc(reg, KB * 8);        
        get_level1_stats(reg, &level1_count_now, &level1_size_now);                
        get_level2_stats(reg, &level2_count_now);
        
        test_not_null(test, "alloc small: got 2 ", buffer[1]);   
        test_equal(test, "alloc small: got 2, size 8KB", 8 * KB, MSIZE(buffer[1]));
        
        test_equal(test, "alloc small: got 2, level 1 size unchanged", 
                   level1_size_now, level1_size_org);
        
        test_equal(test, "alloc small: got 2, level 2 count unchanged", 
                   level2_count_now, level2_count_org);      
                        
        
        /* gets 512 KB from a new l2 */
        buffer[2] = mreg_alloc(reg, 257 * KB);  /* size alignment: 257 => 512 */
        get_level1_stats(reg, &level1_count_now, &level1_size_now);                
        get_level2_stats(reg, &level2_count_now);
        
        test_not_null(test, "alloc small: got 3 ", buffer[2]);   
        test_equal(test, "alloc small: got 3, size 512KB", 512 * KB, MSIZE(buffer[2]));
        
        test_equal(test, "alloc small: got 3, level 1 size -1MB", 
                   level1_size_now, level1_size_org - MB);
        
        test_equal(test, "alloc small: got 3, level 2 count + 1", 
                   level2_count_now, level2_count_org + 1);                         
        

        /* gets 1024 KB from a new l2 */
        buffer[3] = mreg_alloc(reg, 513 * KB);  /* size alignment: 513 => 1024 */
        get_level1_stats(reg, &level1_count_now, &level1_size_now);                
        get_level2_stats(reg, &level2_count_now);
        
        test_not_null(test, "alloc small: got 4 ", buffer[3]);   
        test_equal(test, "alloc small: got 4, size 1024KB", MB, MSIZE(buffer[3]));
        
        test_equal(test, "alloc small: got 4, level 1 size -2MB", 
                   level1_size_now, level1_size_org - 2 * MB);
        
        test_equal(test, "alloc small: got 4, level 2 count +2", 
                   level2_count_now, level2_count_org + 2);         
                
    }
    test_group_end(test);
    
    
    test_group_start(test, "free (small)");
    {                
        mreg_free(reg, buffer[0]);
        mreg_free(reg, buffer[1]);
        mreg_free(reg, buffer[2]);        
        mreg_free(reg, buffer[3]);
                
        check_restored(test, reg);
    }
    test_group_end(test);
            
    
    test_group_start(test, "alloc/free (small+ large)");
    {   
        /* buffer[2] steals 1MB which means buffer[3] cannot be allocated */
        buffer[0] = mreg_alloc(reg, KB * 8);        
        buffer[1] = mreg_alloc(reg, KB * 8);        
        buffer[2] = mreg_alloc(reg, KB * 4);        
        buffer[3] = mreg_alloc(reg, MB * 3);              
                
        test_not_null(test, "alloc/free: got 1 ", buffer[0]);   
        test_not_null(test, "alloc/free: got 2 ", buffer[1]);   
        test_not_null(test, "alloc/free: got 3 ", buffer[2]);   
        test_null(test, "alloc/free: didn't get 4 ", buffer[3]);                
        
        /* if we free buffer[2], 3MB can be allocated */
        mreg_free(reg, buffer[2]);
        buffer[2] = mreg_alloc(reg, MB * 3);
        test_not_null(test, "alloc/free: got 3 (3MB) ", buffer[2]);   
        
        /* clean up and check if its all gone */
        mreg_free(reg, buffer[0]);
        mreg_free(reg, buffer[1]);
        mreg_free(reg, buffer[2]);        
        check_restored(test, reg);
    }
    test_group_end(test);
    
    test_group_start(test, "alloc at");
    {       
        buffer[0] = mreg_alloc_at(reg, MB, KB);  /* allocates 1MB-1MB+4KB */                
        buffer[1] = mreg_alloc_at(reg, MB * 2, MB);
        buffer[2] = mreg_alloc_at(reg, MB + 1, KB); /* already taken by buffer[0]! */
        buffer[3] = mreg_alloc_at(reg, 0, KB);
        
        test_not_null(test, "alloc at: got 1 ", buffer[0]);
        test_not_null(test, "alloc at: got 2 ", buffer[1]);
        test_null(test, "alloc at: didn't get 3 ", buffer[2]);
        test_null(test, "alloc at: didn't get 3 ", buffer[3]);
                
        /* clean up and check if its all gone */
        mreg_free(reg, buffer[0]);
        mreg_free(reg, buffer[1]);        
        check_restored(test, reg);        
    }
    test_group_end(test);
        
    test_group_start(test, "attach multiple");
    {    
        reg = mreg_create(PAGE_BITS);        
        test_not_null(test, "sanity check: allocated another mreg", reg);        
        
        mreg_region_attach(reg, MB, MB + 8 * KB);        
        mreg_region_attach(reg, 0, MB);        
        mreg_region_attach(reg, 4 * MB, MB + 16 * KB);
        
        get_level1_stats(reg, &level1_count_now, &level1_size_now);
        get_level2_stats(reg, &level2_count_now);
        
        test_equal(test, "atach multi: total start", 0, reg->start);
        test_equal(test, "atach multi: total end",  5 * MB + 16 * KB-1, reg->end);        
        test_equal(test, "attach multi: level1 count", 2, level1_count_now); /* 0MB & 1MB are combined */
        test_equal(test, "attach multi: level1 size", 3 * MB, level1_size_now);
        test_equal(test, "attach multi: level2 count", 2, level2_count_now);
        
        /* see if the layout is as we expect it */
        buffer[0] = mreg_alloc(reg, 16 * KB);
        buffer[1] = mreg_alloc(reg, 8 * KB);
        
        test_not_null(test, "attach multi: got small 16KB", buffer[0]);
        test_not_null(test, "attach multi: got small 8KB", buffer[1]);
        test_equal(test, "attach multi: 16KB at correct location", 5 * MB, buffer[0]->start);
        test_equal(test, "attach multi: 8KB at correct location", 2 * MB, buffer[1]->start);        
        
        /* now we should be able to allocate the 2 MB at 0 and the 1 MB at 5 */
        buffer[2] = mreg_alloc(reg, 2 * MB);
        buffer[3] = mreg_alloc(reg, 1 * MB);
        buffer[4] = mreg_alloc(reg, 1 * KB);
        
        test_not_null(test, "attach multi: got large 2MB", buffer[2]);
        test_not_null(test, "attach multi: got large 1MB", buffer[3]);
        test_null(test, "attach multi: didn't get small 1KB", buffer[4]);
        
        test_equal(test, "attach multi: 2MB at correct location", 0 * MB, buffer[2]->start);
        test_equal(test, "attach multi: 2MKB at correct location", 4 * MB, buffer[3]->start);
        
    }
    test_group_end(test);    
    
    /* test stuff that end at 0x0000_0000 (wrap around) */
    test_group_start(test, "upper border regions");
    {    
        reg = mreg_create(PAGE_BITS);   
        test_not_null(test, "sanity check: allocated another mreg", reg);        
         
        mreg_region_attach(reg, 4095 * MB, MB);
        get_level1_stats(reg, &level1_count_now, &level1_size_now);
        get_level2_stats(reg, &level2_count_now);
        
        test_equal(test, "upper border regions: level1 count", 1, level1_count_now);
        test_equal(test, "upper border regions: level1 size", 1 * MB, level1_size_now);
        test_equal(test, "upper border regions: level1 count", 0, level2_count_now);
        
        /* allocate a normal 1MB */
        buffer[0] = mreg_alloc(reg, MB);        
        test_not_null(test, "upper border regions: got 1MB", buffer[0]);
        mreg_free(reg, buffer[0]);
        
        /* allocate at 1MB */
        buffer[0] = mreg_alloc_at(reg, 4095 * MB, MB);        
        test_not_null(test, "upper border regions: got 1MB at 4095MB", buffer[0]);
        mreg_free(reg, buffer[0]);
        
        
        /* allocate one page in the middle */
        buffer[0] = mreg_alloc_at(reg, 0xFFFF0000, PAGE_SIZE);
        test_not_null(test, "Got one page at 0xFFFF0000", buffer[0]);
        test_equal(test, "Correct page start", 0xFFFF0000, buffer[0]->start);
        test_equal(test, "Correct page start", 0xFFFF0000 + PAGE_SIZE - 1, buffer[0]->end);        
    }
    test_group_end(test);    
    

    /* test stuff that starts at 0x0000_0000  */
    test_group_start(test, "lower border regions");
    {    
        
        /* type 1: under 1MB */
        reg = mreg_create(PAGE_BITS);   
        test_not_null(test, "sanity check: allocated another mreg", reg);        
         
        mreg_region_attach(reg, 0, 2 * PAGE_SIZE);
        get_level1_stats(reg, &level1_count_now, &level1_size_now);
        get_level2_stats(reg, &level2_count_now);
        
        test_equal(test, "lower border regions: level1 count", 0, level1_count_now);
        test_equal(test, "lower border regions: level1 size", 0, level1_size_now);
        test_equal(test, "lower border regions: level1 count", 1, level2_count_now);
        
        buffer[0] = mreg_alloc(reg, PAGE_SIZE);
        buffer[1] = mreg_alloc(reg, PAGE_SIZE);
        buffer[2] = mreg_alloc(reg, PAGE_SIZE);
        
        test_not_null(test, "lower border regions: got page 1", buffer[0]);
        test_not_null(test, "lower border regions: got page 2", buffer[1]);
        test_null(test, "lower border regions: didn't get page 3", buffer[2]);
        
        
        /* type 2: exactly 1MB */
        reg = mreg_create(PAGE_BITS);   
        test_not_null(test, "sanity check: allocated another mreg", reg);        
         
        mreg_region_attach(reg, 0, MB);
        get_level1_stats(reg, &level1_count_now, &level1_size_now);
        get_level2_stats(reg, &level2_count_now);
        
        test_equal(test, "lower border regions (2): level1 count", 1, level1_count_now);
        test_equal(test, "lower border regions (2): level1 size", MB, level1_size_now);
        test_equal(test, "lower border regions (2): level1 count", 0, level2_count_now);
        
        buffer[0] = mreg_alloc(reg, MB);
        buffer[1] = mreg_alloc(reg, PAGE_SIZE);        
        test_not_null(test, "lower border regions (2): got page 1", buffer[0]);
        test_null(test, "lower border regions (2): didn't get page 2", buffer[1]);        
        mreg_free(reg, buffer[0]);
        mreg_free(reg, buffer[1]);
        
        buffer[0] = mreg_alloc(reg, MB / 2);
        buffer[1] = mreg_alloc(reg, MB / 2);
        buffer[2] = mreg_alloc(reg, PAGE_SIZE);        
        test_not_null(test, "lower border regions (2B): got page 1", buffer[0]);
        test_not_null(test, "lower border regions (2B): got page 2", buffer[1]);
        test_null(test, "lower border regions (2B): didn't get page 3", buffer[2]);
        
        
        /* type 3: just over 1MB */
        reg = mreg_create(PAGE_BITS);   
        test_not_null(test, "sanity check: allocated another mreg", reg);        
                
        mreg_region_attach(reg, 0, MB + PAGE_SIZE);
        get_level1_stats(reg, &level1_count_now, &level1_size_now);
        get_level2_stats(reg, &level2_count_now);
        
        test_equal(test, "lower border regions (3): level1 count", 1, level1_count_now);
        test_equal(test, "lower border regions (3): level1 size", MB, level1_size_now);
        test_equal(test, "lower border regions (3): level1 count", 1, level2_count_now);
        
        buffer[0] = mreg_alloc(reg, MB);
        buffer[1] = mreg_alloc(reg, PAGE_SIZE);        
        buffer[2] = mreg_alloc(reg, PAGE_SIZE);        
        
        test_not_null(test, "lower border regions (3): got page 1", buffer[0]);
        test_not_null(test, "lower border regions (3): got page 2", buffer[1]);        
        test_null(test, "lower border regions (3): didn't get page 3", buffer[2]);        
        mreg_free(reg, buffer[0]);
        mreg_free(reg, buffer[1]);
        mreg_free(reg, buffer[2]);
        
        
        buffer[0] = mreg_alloc(reg, MB / 2);
        buffer[1] = mreg_alloc(reg, MB / 2);
        buffer[2] = mreg_alloc(reg, PAGE_SIZE);        
        buffer[3] = mreg_alloc(reg, PAGE_SIZE);        
        
        test_not_null(test, "lower border regions (3B): got page 1", buffer[0]);
        test_not_null(test, "lower border regions (3B): got page 2", buffer[1]);
        test_not_null(test, "lower border regions (3B): got page 3", buffer[2]);
        test_null(test, "lower border regions (3B): didn't get page 4", buffer[3]);        
    }
    test_group_end(test);    

    
    
    
    /* 
     * stress testing 
     */    
    test_group_start(test, "alloc: stress test");
    {            
        /* need a larger one */
        reg = mreg_create(PAGE_BITS);        
        test_not_null(test, "sanity check: allocated a larger mreg", reg);        
        
        mreg_region_attach(reg, MB - KB * 8, MB * 512 + KB * 16);        
        get_level1_stats(reg, &level1_count_org, &level1_size_org);
        get_level2_stats(reg, &level2_count_org);
        
        
        /* allocate a lot and free them at random order */
        for(j = 0; j < 10240; j++) {            
            for(i = 0; i < COUNT; i++) {
                size_t size = 4096 * (1 + (rand() & 4095));
                buffer[i] = mreg_alloc(reg, size);
            }
            
            for(i = 0; i < COUNT; i++) {
                int r = rand() % COUNT;
                memregion_t *tmp = buffer[i];
                buffer[i] = buffer[r];
                buffer[r] = tmp;
            }            
            
            for(i = 0; i < COUNT; i++) {
                mreg_free(reg, buffer[i]);
            }            
        }
        
        check_restored(test, reg);                
    }
    test_group_end(test);
        
    
    test_group_start(test, "alloc at: stress test");
    {            
        for(j = 0; j < 10240; j++) {            
            for(i = 0; i < COUNT; i++) {
                size_t size = PAGE_SIZE * (1 + (rand() & 4095));
                addr_t adr   = reg->start + (rand() % (reg->end - reg->start));
                adr &= ~ PAGE_SIZE;
                buffer[i] = mreg_alloc(reg, size);
            }
            
      
            for(i = 0; i < COUNT; i++) {
                int r = rand() % COUNT;
                memregion_t *tmp = buffer[i];
                buffer[i] = buffer[r];
                buffer[r] = tmp;
            }            
                        
            for(i = 0; i < COUNT; i++) {
                mreg_free(reg, buffer[i]);
            }            
        }
        
        check_restored(test, reg);                
    }
    test_group_end(test);    
}
