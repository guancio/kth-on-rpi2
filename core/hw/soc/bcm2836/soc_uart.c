#include "hw.h"

#define USART_BUFFER_SIZE 16

//Ah - this is familiar. I can probably rewrite this pretty easily.

static char buffer_out[USART_BUFFER_SIZE];
static char buffer_in[USART_BUFFER_SIZE];

static usart_registers *usart0 = 0;

void stdio_write_char(int c)
{
    if(!usart0) return;
    
    while(usart0->tcr != 0)
        ;
    
    buffer_out[0] = c;
	//Brutal but not effective?
    usart0->tpr = (uint32_t)GET_PHYS(buffer_out);
    usart0->tcr = 1;
}
extern int stdio_read_char()
{
    return -1; /* TODO */    
}

extern int stdio_can_write()
{
    if(!usart0) return 1; /* return 1 to avoid infinite wait before init*/
    
    return (usart0->tcr == 0) ? 1 : 0;            
}
extern int stdio_can_read()
{
    return 0; /* TODO */
}

/**********************************************************************/
void soc_uart_init()
{
#if 0
    memspace_t *ms_uart;
    
    
    ms_uart = env_map_from(PROC_TYPE_HYPERVISOR, PROC_TYPE_HYPERVISOR,
                           "__soc_usart", USART0_BASE, sizeof(usart_registers) , TRUE);
        
    usart0 = (usart_registers *) ms_uart->vadr;

    /* reset them, disable RX, TX & interrupts */
    usart0->cr = 0xAC;    
    usart0->idr =-1;
    
    /* set mode: 8/1/N, MCLK/8 */
    usart0->mr = 0x9D0;
    
    /* set baud rate */
    /* XXX: MCLK should be changed to board_get_system_lock() or similair */
    usart0->brgr = (20000000 / 16 / 115200);
    
    /* set buffers */
    usart0->rpr = (uint32_t)GET_PHYS( buffer_in );
    usart0->tpr = (uint32_t)GET_PHYS( buffer_out );
    usart0->tcr = 0;
    usart0->rcr = 0;
    
    /* now enable in/out */
    usart0->cr = 0x050;        
#endif
}
    
