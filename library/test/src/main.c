
#include <test.h>

#ifdef COMPILE_HOST
extern void test_do(test_context_t *);

int main()
{
    test_context_t ctxt;
    test_init( &ctxt);
    
    test_do(&ctxt);
    test_cleanup( &ctxt);    
    
    return 0;
}
#endif
