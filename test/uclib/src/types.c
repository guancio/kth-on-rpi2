
#include <uclib.h>
#include <test.h>

#define SET_SIZE 100

void test_do(test_context_t *test)
{
    
    
    test_mark(test, "minic types");
    test_group_start(test, "type sizes");
    {
        
        test_equal(test, "uint8_t is 8 bit",   1, sizeof(uint8_t));
        test_equal(test, "uint16_t is 16 bit", 2, sizeof(uint16_t));
        test_equal(test, "uint32_t is 32 bit", 4, sizeof(uint32_t));
        test_equal(test, "addr_t is 32 bit",    4, sizeof(addr_t));
               
    }
    test_group_end(test);

}
