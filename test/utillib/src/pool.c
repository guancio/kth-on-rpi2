
#include <utillib.h>
#include <test.h>

#define ITEM_COUNT 10

typedef struct {
    int data1;
    int data2;
} my_data_t;

void test_do(test_context_t *test)
{
    pool_ctxt_t pool;
    my_data_t *tmp;
    int i;
    void *buffer[ITEM_COUNT];
    
    
    
    test_mark(test, "pool functions");
    
    test_group_start(test, "Setup and sanity check");
    {
        int ret = pool_init( &pool, sizeof(my_data_t), ITEM_COUNT);
        test_not_equal(test, "could init pool", ret, 0);
        
    }
    test_group_end(test); 
    
    test_group_start(test, "get pool items");
    {        
        for(i = 0; i < ITEM_COUNT; i++) {
            buffer[i] = pool_alloc(&pool);
            test_not_null( test, "getting items from pool", buffer[i] );
            if(buffer[i]) {
                tmp = (my_data_t *) buffer[i];
                tmp->data1 = i;
                tmp->data2 = i;
            }
        }        
        tmp = pool_alloc(& pool);
        test_null( test, "Pool must be empty", tmp);
    }
    test_group_end(test);    
    
    test_group_start(test, "return pool items");
    {
        my_data_t *tmp1, *tmp2, *tmp3, *tmp4;
        
        /* return two items to pool */
        tmp1 = buffer[0];
        tmp2 = buffer[1];
        
        pool_free( &pool, tmp1);
        pool_free( &pool, tmp2);
        
        /* now when we allocate we must get the same two back */
        tmp3 = pool_alloc(& pool);
        tmp4 = pool_alloc(& pool);
        
        if( (tmp1 == tmp3 && tmp2 == tmp4) || (tmp1 == tmp4 && tmp2 == tmp3) ) {
            /* got them ! */
        } else {
            test_fail(test, "Did not get the freed items back");
        }
        
        tmp = pool_alloc(& pool);
        test_null( test, "Pool must be empty (again)", tmp);        
    }
        
    test_group_end(test);        
}

