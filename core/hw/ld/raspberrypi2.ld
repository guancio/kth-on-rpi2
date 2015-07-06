__page_size__ = 0x1000; /* 4KiB page size */
//TODO: Is this the address where we start inserting the program into RAM? Should be immediately after U-Boot if using U-Boot or at 0x0 if not using U-Boot.
__ram_start__ = 0x80000000;
//TODO: Is this the address where we stop inserting the program into RAM?
__ram_end__   = 0x88000000;
//This looks to be overwritten later on...
__hyperstack_top__ = 0x100000 - 4; /* Hypervisor stack located at end of hypervisor address */

MEMORY 
{
    RAM : org = 0x8000C000, l = 127M
}

ENTRY(_start) 
 
SECTIONS
{
    . = __ram_start__;
    __hyper_start__ = .;
    
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
        __hyper_bss_start__ = .;
        *.o (.bss)
        *.to(.bss)
        __hyper_bss_end__ = .;
    } > RAM
    
    /* heap: up to 104K */    
    __hyper_heap_start__ = .;    
    . = 0xf0100000 - 1024 * (64 + 32 + 8);
    __hyper_heap_end__ = .;
    
    /* stack */
    __hyper_stack_bottom__ = .;
    . += 1024 * 8;
    __hyper_stack_top__ = .;         

    /* guest */
    __hyper_guest_start__ = .;
    . += 1024 * 32;
    __hyper_guest_end__ = .;     
    
     /* PT memory */    
    . = ALIGN(__page_size__);
    __hyper_pt_start__ = .;
    . += 1024 * 64;    
    __hyper_pt_end__ = .;

    __hyper_end__ = .;        
    
}
