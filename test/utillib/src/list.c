
#include <utillib.h>
#include <test.h>


struct data {
    void *next;
    void *prev;
    int data;
};

/* ----------------------------------------------- */

void test_do(test_context_t *test)
{    
    list_t list;
    struct data data1, data2, data3;
    
    test_mark(test, "list");
    
    test_group_start(test, "Setup and sanity check");
    {
        list_init(&list);
        
        test_equal(test, "Sanity check: initial list is empty",
                   TRUE, list_is_empty(&list) );
        
        test_equal(test, "Sanity check: initial size is zero",
                   0, list_size_get(&list) );
    }
    test_group_end(test);
    
    
    test_group_start(test, "add / find");
    {   
        
        test_equal(test, "initial: not found 1", FALSE, list_find(&list, & data1));
        test_equal(test, "initial: not found 2", FALSE, list_find(&list, & data2));
        test_equal(test, "initial: not found 3", FALSE, list_find(&list, & data3));
        
        
        
        list_add(&list, &data1);
        test_equal(test, "added 1: not empty", FALSE, list_is_empty(&list) );        
        test_equal(test, "added 1: size = 1", 1, list_size_get(&list) );
        
        test_equal(test, "added 1: found 1", TRUE, list_find(&list, & data1));
        test_equal(test, "added 1: not found 2", FALSE, list_find(&list, & data2));
        test_equal(test, "added 1: not found 3", FALSE, list_find(&list, & data3));
        
        
        list_add(&list, &data2);
        test_equal(test, "added 2: not empty", FALSE, list_is_empty(&list) );        
        test_equal(test, "added 2: size = 2", 2, list_size_get(&list) );
        
        test_equal(test, "added 2: found 1", TRUE, list_find(&list, & data1));
        test_equal(test, "added 2: found 2", TRUE, list_find(&list, & data2));
        test_equal(test, "added 2: not found 3", FALSE, list_find(&list, & data3));
        
        list_add(&list, &data3);
        test_equal(test, "added 3: not empty", FALSE, list_is_empty(&list) );        
        test_equal(test, "added 3: size = 3", 3, list_size_get(&list) );
        
        test_equal(test, "added 3: found 1", TRUE, list_find(&list, & data1));
        test_equal(test, "added 3: found 2", TRUE, list_find(&list, & data2));
        test_equal(test, "added 3: found 3", TRUE, list_find(&list, & data3));
        
    }
    test_group_end(test);
    
    
    test_group_start(test, "remove / find");
    {           
        list_remove(&list, &data2);
        test_equal(test, "removed 2: size = 2", 2, list_size_get(&list) );
        test_equal(test, "removed 2: is not empty", FALSE, list_is_empty(&list) );        
        
        test_equal(test, "removed 2: found 1", TRUE, list_find(&list, & data1));
        test_equal(test, "removed 2: not found 2", FALSE, list_find(&list, & data2));
        test_equal(test, "removed 2: found 3", TRUE, list_find(&list, & data3));
        
        list_remove(&list, &data1);
        test_equal(test, "removed 1: size = 1", 1, list_size_get(&list) );
        test_equal(test, "removed 1: is not empty", FALSE, list_is_empty(&list) );        
        
        test_equal(test, "removed 1: not found 1", FALSE, list_find(&list, & data1));
        test_equal(test, "removed 1: not found 2", FALSE, list_find(&list, & data2));
        test_equal(test, "removed 1: found 3", TRUE, list_find(&list, & data3));
        
        
        list_remove(&list, &data3);
        test_equal(test, "removed 3: size = 0", 0, list_size_get(&list) );
        test_equal(test, "removed 3: is empty", TRUE, list_is_empty(&list) );        
        
        test_equal(test, "removed 3: not found 1", FALSE, list_find(&list, & data1));
        test_equal(test, "removed 3: not found 2", FALSE, list_find(&list, & data2));
        test_equal(test, "removed 3: not found 3", FALSE, list_find(&list, & data3));                
    }
    test_group_end(test);
    
    
    test_group_start(test, "pop");
    {     
        list_t *tmp1, *tmp2, *tmp3;
        
        test_equal(test, "empty: cannout pop", 0, list_pop(&list));
        
        list_add(&list, &data2);
        list_add(&list, &data1);
        
        tmp1 = list_pop(&list);
        tmp2 = list_pop(&list);
        tmp3 = list_pop(&list);
        
        
        test_not_equal(test, "has 2: first pop not NULL", 0, tmp1);
        test_not_equal(test, "has 1: second pop not NULL", 0, tmp2);
        test_not_equal(test, "popped 2: first and second not equal", tmp1, tmp2);        
        test_equal(test, "empty again: third pop returned NULL", 0, tmp3);
        test_equal(test, "empty again: size = 0", 0, list_size_get(&list) );
        
    }
    test_group_end(test);
        
    
    test_group_start(test, "error handling");
    {     
        list_add(&list, &data1);
        list_add(&list, &data2);        
        list_add(&list, &data1);        
        test_equal(test, "error: item added only once", 2, list_size_get(&list) );
        
        
        list_remove(&list, &data1);
        list_add(&list, &data3);
        test_equal(test, "(sanity check) error: item removed, another one added", 2, list_size_get(&list) );
        
        
        
        list_remove(&list, &data1);
        list_add(&list, &data3);
        test_equal(test, "error: repeating add/remove has no results", 2, list_size_get(&list) );
        
        
    }
    test_group_end(test);
    
}
