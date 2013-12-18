
#include <utillib.h>
#include <test.h>


/* bit feild */
#define TEST_BF1_SHIFT 0
#define TEST_BF1_MASK  7

#define TEST_BF2_SHIFT 3
#define TEST_BF2_MASK  31

#define TEST_BF3_SHIFT 8
#define TEST_BF3_MASK  3


void test_do(test_context_t *test)
{
    
    int i, j, k;
    
    
    test_mark(test, "macros");
    test_group_start(test, "EXTRACT/INSERT");
    {
        for(i = 0; i <= TEST_BF1_MASK; i++) {
            for(j = 0; j <= TEST_BF2_MASK; j++) {
                for(k = 0; k <= TEST_BF3_MASK; k++) {
                    int val =  BF_INSERT(TEST_BF1, 0, i);
                    val =  BF_INSERT(TEST_BF2, val, j);
                    val =  BF_INSERT(TEST_BF3, val, k);

                    test_equal(test, "bf1 test", i, BF_EXTRACT(TEST_BF1, val));
                    test_equal(test, "bf2 test", j, BF_EXTRACT(TEST_BF2, val));
                    test_equal(test, "bf3 test", k, BF_EXTRACT(TEST_BF3, val));
                    
                }
            }
        }               
    }
    test_group_end(test);
    
    
    test_group_start(test, "ALIGN_UP/DOWN");
    {
        test_equal(test, "align down (1)", 0 * 4096, ALIGN_DOWN(0, 4096));
        test_equal(test, "align down (2)", 0 * 4096, ALIGN_DOWN(4095, 4096));
        test_equal(test, "align down (3)", 1 * 4096, ALIGN_DOWN(4096, 4096));
        test_equal(test, "align down (4)", 1 * 4096, ALIGN_DOWN(4097, 4096));
        
        
        test_equal(test, "align up (1)", 0 * 4096, ALIGN_UP(0, 4096));
        test_equal(test, "align up (2)", 1 * 4096, ALIGN_UP(4095, 4096));
        test_equal(test, "align up (3)", 1 * 4096, ALIGN_UP(4096, 4096));
        test_equal(test, "align up (4)", 2 * 4096, ALIGN_UP(4097, 4096));
        
    }
    test_group_end(test);

}
