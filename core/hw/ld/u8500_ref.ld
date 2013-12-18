__page_size__ = 0x1000; /*4KB page size*/
__ram_start__ = 0x00000000;
__ram_end__   = 0x01000000;
__hyperstack_top__ = 0x100000 - 4; /* Hypervisor stack located at end of hypervisor address*/

MEMORY 
{
    RAM : org = 0x00000000, l = 0x01000000
}

ENTRY(_start) 
 
SECTIONS
{
    . = __ram_start__;
    
    /* code: startup first */
    .text : {
        __startup_start__ = .;      
        *(.startup)
        __startup_end__ = .;
        __code_start__ = .;
        *.o (.text)
        *.to(.text)
        __code_end__ = .;
        
    } > RAM
        
    /* DATA */
    .data : {
        __data_start__ = .;        
        *.o (.data)
        *.to(.data)
        __data_end__ = .;
    } > RAM

    /* BSS */
    .bss ALIGN(__page_size__) : {
        __bss_start__ = .;
        *.o (.bss)
        *.to(.bss)
        __bss_end__ = .;
    } > RAM
    
    . = ALIGN(__page_size__);
    __heap_start__ = .;
}
