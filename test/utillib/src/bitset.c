
#include <utillib.h>
#include <test.h>

#define SET_SIZE 100

void test_do(test_context_t *test)
{
    
    bitset_t *set;
    int i;
    
    
    test_mark(test, "bitset functions");
    test_group_start(test, "Setup and sanity check");
    {
        
        set = bitset_init(SET_SIZE);
        test_not_null(test, "Sanity check: allocated bitset", set);
               
    }
    test_group_end(test);
    
    
    test_group_start(test, "set / get");
    {
        int set1[6] = { 0, 1, 3, 5, SET_SIZE -1, SET_SIZE - 3 };
        int set2[6] = { 2, 4, 6, 10, 11, SET_SIZE - 2 };
        
        /* set1 is TRUE, set2 is false */
        for(i = 0; i < 6; i++) bitset_set(set, set1[i], TRUE);
        for(i = 0; i < 6; i++) bitset_set(set, set2[i], FALSE);
        
        /* now check it */        
        for(i = 0; i < 6; i++) {
            test_equal(test, "set1 members are TRUE", bitset_get(set, set1[i]), TRUE);
            test_equal(test, "set2 members are FALSE", bitset_get(set, set2[i]), FALSE);
        }
        
        /* reverse it and try it again */
        for(i = 0; i < 6; i++) bitset_set(set, set1[i], FALSE);
        for(i = 0; i < 6; i++) bitset_set(set, set2[i], TRUE);
        
        for(i = 0; i < 6; i++) {
            test_equal(test, "set1 members are FALSE (2)", bitset_get(set, set1[i]), FALSE);
            test_equal(test, "set2 members are TRUE  (2)", bitset_get(set, set2[i]), TRUE);
        }
    }
    test_group_end(test);
    
 
    test_group_start(test, "set all");
    {
        
        /* ALL TRUE */
        bitset_set_all(set, TRUE);
        bitset_set(set, 5, FALSE); /* special one to catch stuck-on bugs ? */
        
        for(i = 0; i < SET_SIZE; i++) {
            if(i != 5)
                test_equal(test, "set_all (TRUE)", bitset_get(set, i), TRUE);
            else
                test_equal(test, "set_all (one is FALSE)", bitset_get(set, i), FALSE);
        }
        
        /* ALL FALSE */
        bitset_set_all(set, FALSE);
        bitset_set(set, 31, TRUE); /* see above */
        
        for(i = 0; i < SET_SIZE; i++) {
            if(i != 31)
                test_equal(test, "set_all (FALSE)", bitset_get(set, i), FALSE);
            else
                test_equal(test, "set_all (one is TRUE)", bitset_get(set, i), TRUE);
        }
        
        
    }
    test_group_end(test);    

}
