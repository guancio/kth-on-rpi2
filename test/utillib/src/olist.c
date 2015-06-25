
#include <utillib.h>
#include <test.h>


/* 
 * some helper functions 
 */

/* push items 1...n to the list */
BOOL push_items(olist_t *list, int n)
{
    int i;
    BOOL b = TRUE;
    
    for(i = 1; i <= n; i++) 
        b &= olist_item_push(list, (void *) i);
    
    return b;
}

/* return a bitmap for the found items, works only for n < 32 */
uint32_t get_bitmap(olist_t *list)
{
    int i;
    uint32_t ret = 0;
    
    for(i = 0; i < olist_size_get(list); i++) {
        uint32_t x = (uint32_t) olist_item_get(list, i);
        if(x == 0 || x > 31) continue;
        ret |= 1UL << x;
    }
    
    return ret;
}

/* remove all items from the list */
void remove_all(olist_t *list)
{
    while( olist_size_get(list) > 0)
        olist_item_pop(list);    
}

/* ----------------------------------------------- */
void test_do(test_context_t *test)
{    
    olist_t list;
    BOOL b;
    int i, j;
    void *tmp;
    
    test_mark(test, "object list");
    test_group_start(test, "Setup and sanity check");
    {
        b = olist_init(&list);
        test_equal(test, "Sanity check: allocated olist", TRUE, b);
        
        test_equal(test, "Sanity check: initial size is zero",
                   0, olist_size_get(&list) );
    }
    test_group_end(test);
    
    
    test_group_start(test, "push / get / pop");
    {
        /* push three items */
        b = push_items(&list, 3);
        test_equal(test, "push: added 1-3", TRUE, b);        
        test_equal(test, "push: has now 3 items",
                   3, olist_size_get(&list) );
        
        /* see if they are all in there */
        i = get_bitmap(& list);
        test_equal(test, "get: saw 1-3", 2 + 4 + 8, i);
        
        
        /* pop them out again */
        i = 0;
        for(j = 0; j < 3; j++) {
            tmp = olist_item_pop(&list);            
            i |= 1UL << ((uint32_t) tmp);
            test_not_null(test, "pop: item is not null", tmp);            
        }
                
        /* see if we got them all back */
        test_equal(test, "pop: poped 1-3", 2 + 4 + 8, i);
        test_equal(test, "pop: has now 0 items",
                   0, olist_size_get(&list) );
    }
    test_group_end(test);
    
    
    test_group_start(test, "remove");
    {
        /* push 5 items */
        b = push_items(&list, 5);
        j = get_bitmap(&list);
        
        /* remove the first one */
        tmp = olist_item_get(&list, 0);
        olist_item_remove(&list, 0);
        
        /* see what is left */
        test_equal(test, "remove: has now 4 items", 4, olist_size_get(&list) );
        
        i = get_bitmap(& list);
        test_equal(test, "remove: item 0 no longer in there",
                   i, j & ~ (1UL << (uint32_t) tmp) );
        
        
        /* remove the last one and repeat this again */
        tmp = olist_item_get(&list, 3);
        olist_item_remove(&list, 3);        
        j = get_bitmap(& list);
        
        test_equal(test, "remove: has now 3 items", 3, olist_size_get(&list) );        
        test_equal(test, "remove: item 3 no longer in there",
                   j, i & ~ (1UL << (uint32_t) tmp) );        
        
        
        /* remove the middle one and repeat this again */
        tmp = olist_item_get(&list, 1);
        olist_item_remove(&list, 1);        
        i = get_bitmap(& list);
        
        test_equal(test, "remove: has now 2 items", 2, olist_size_get(&list) );        
        test_equal(test, "remove: item 2 no longer in there",
                   i, j & ~ (1UL << (uint32_t) tmp) );        
                
    }
    test_group_end(test);
    
    
    test_group_start(test, "grow");
    {
        /* clean up and push a lot of stuff */
        remove_all(&list);
        b = push_items(&list, 1024 * 1024 * 10);
        
        
        test_equal(test, "grow: added 10M items", TRUE, b);        
        test_equal(test, "grow: has 10M items", 
                   1024 * 1024 * 10, olist_size_get(&list) );
        
        /* see if they are ALL in there: */
        {
            bitset_t *set = bitset_init( 1024 * 1024 * 10 + 1);
            test_not_null(test, "grow: allocated bitset for this test", set);
            
            bitset_set_all(set, FALSE);
            for(i = 0; i < olist_size_get(&list); i++) {
                tmp = olist_item_get(&list, i);
                test_not_null(test, "grow: item was not null", tmp);
                
                j = (uint32_t) tmp;
                                
                test_larger_than(test, "grow: item > 0", j, 0);
                test_less_or_equal(test, "grow: item <= 10M", j, 1024 * 1024 * 10);
                test_equal(test, "grow: item not seen before",
                           FALSE, bitset_get(set, j));
                
                bitset_set(set, j, TRUE); /* mark it for next time */
            }
            
            bitset_cleanup(set);
        }
        
    }
    test_group_end(test);
    
    test_group_start(test, "invalid access");
    {
        /* create four items and remove one */
        remove_all(&list);
        b = push_items(&list, 4);
        
        /* cannot outside buffer */
        tmp = olist_item_get(&list, -1);
        test_null(test, "invalid: item -1 is outside valid range", tmp);
        
        tmp = olist_item_get(&list, 4);
        test_null(test, "invalid: item 4 is outside valid range", tmp);
        
        /* item 3 will be invalid when removed */
        tmp = olist_item_get(&list, 3);
        test_not_null(test, "invalid: item 3 is within valid range", tmp);
        
        olist_item_pop(&list);
        tmp = olist_item_get(&list, 3);
        test_null(test, "invalid: item 3 is now outside valid range", tmp);                
    }
    test_group_end(test);


}
