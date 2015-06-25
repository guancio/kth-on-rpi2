__page_size__ = 0x1000; /*4KB page size*/

MEMORY
{
    RAM : org = 0xf0500000, l = 127M
}

ENTRY(_start)

SECTIONS
{
    . = 0xf0500000;

	.startup : {
		__startup_start__ = .;
        *(.startup)
        __startup_end__ = .;
		
	} > RAM

    .text : {
        __code_start__ = .;
        *.o (.text)
        __code_end__ = .;
    } > RAM

    .bss ALIGN(__page_size__) : {
        __bss_start__ = .;
        *.o (.bss)
        __bss_end__ = .;
    } > RAM

    .data : {
        __data_start__ = .;
        *.o (.data)
        __data_end__ = .;
    } > RAM

    /* heap: up to 104K */    
    __trusted_heap_start__ = .;    
    . = 0xf0600000 - 1024 * (64 + 32 + 8);
    __trusted_heap_end__ = .;
    
    /* stack */
    __stack_bottom__ = .;
    . += 1024 * 8;
    __stack_top__ = .;       


}
