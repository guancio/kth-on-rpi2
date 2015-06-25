
#include <utillib.h>
#include <test.h>

#define ITEM_COUNT 10
#define MAGIC_ALLOC 0x12345679
#define MAGIC_FREE  0x55556666

typedef struct {
    int data1;
    int data2;
} my_data_t;

static int my_user_data = 0;
static test_context_t *test;

/* ------------------------------------------------------ */

void my_constructor (void *obj, void *ud)
{
    my_data_t *data = (my_data_t *) obj;
    
    test_equal(test, "correct userdata in constructor", ud, &my_user_data);
    test_not_equal(test, "not already cosntructed", data->data1,MAGIC_ALLOC);
    data->data1 = MAGIC_ALLOC;
    data->data2 = my_user_data++;
}

void my_destructor (void *obj, void *ud)
{
    my_data_t *data = (my_data_t *) obj;    
    
    test_equal(test, "correct userdata in destructor", ud, &my_user_data);
    test_equal(test, "destructed obj has been consructed", data->data1, MAGIC_ALLOC);
    test_not_equal(test, "not already destructed", data->data1, MAGIC_FREE);
    data->data1 = MAGIC_FREE;
    data->data2 = my_user_data--;
    
}

/* ------------------------------------------------------ */

void test_do(test_context_t *test_)
{
    slab_ctxt_t slab;
    
    my_data_t *tmp;
    int i, ret, count;
    void *buffer[ITEM_COUNT];
    
    /* global test context, needed by slab callbacks above */
    test = test_;
    
    test_mark(test, "slab functions");
    
    test_group_start(test, "Setup and sanity check");
    {
        ret = slab_init( &slab, sizeof(my_data_t), ITEM_COUNT,
                         my_constructor, my_destructor, & my_user_data);
        test_not_equal(test, "could init slab", ret, 0);        
    }
    test_group_end(test); 
    
    test_group_start(test, "get slab items");
    {        
        for(i = 0; i < ITEM_COUNT; i++) {
            tmp = slab_alloc(& slab);
            buffer[i] = tmp;
            test_not_null( test, "getting items from slab", tmp );
            if(tmp) {
                test_equal(test, "allocated obj has been consructed", tmp->data1, MAGIC_ALLOC);                
            }
        }        
        
        count = my_user_data;
        test_larger_or_equal(test, "correct amount of objects constructed", ITEM_COUNT, my_user_data);
        
        tmp = slab_alloc(& slab);
        test_null( test, "slab must be empty", tmp);                
        test_equal(test, "did not construct a new one when slab was empty", count, my_user_data);
    }
    test_group_end(test);    
    
    test_group_start(test, "return slab items");
    {
        my_data_t *tmp1, *tmp2, *tmp3, *tmp4;
        
        /* return two items to slab */
        tmp1 = buffer[0];
        tmp2 = buffer[1];
        
        slab_free( &slab, tmp1);
        slab_free( &slab, tmp2);
        
        /* now when we allocate we must get the same two back */
        tmp3 = slab_alloc(& slab);
        tmp4 = slab_alloc(& slab);
        
        if( (tmp1 == tmp3 && tmp2 == tmp4) || (tmp1 == tmp4 && tmp2 == tmp3) ) {
            /* got them ! */
        } else {
            test_fail(test, "Did not get the freed items back");
        }
        
        /* empty again */
        tmp = slab_alloc(& slab);
        test_null( test, "slab must be empty (again)", tmp);
        
        /* no new constructions */
        test_equal(test, "did not construct new slabs after some free/alloc pairs", count, my_user_data);
    }
    test_group_end(test);
    
    
    test_group_start(test, "destruct slab items");
    {
        slab_cleanup(&slab);
        test_equal(test, "all items destructed", my_user_data, 0);        
    }                    
    test_group_end(test);        
}

