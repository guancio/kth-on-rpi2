
#include <lib.h>
#include <types.h>

#include "hypercalls.h"





void _main()
{
    
    // test hypercall from guest
    ISSUE_HYPERCALL_REG1(HYPERCALL_NEW_PGD, 0x01234567);
    for(;;) ;
}

/*Each guest must provide a handler rpc*/
void handler_rpc(unsigned callNum, void *params)
{
}
