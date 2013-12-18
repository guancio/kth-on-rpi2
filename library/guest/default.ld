__page_size__ = 0x1000; /*4KB page size*/
__ram_start__ = 0xc0000000;
__ram_end__   = 0xc8000000;

MEMORY
{
    RAM : org = 0xc0000000, l = 127M
}

ENTRY(_start)

SECTIONS
{
    . = __ram_start__;

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


    . = ALIGN(__page_size__);
    __heap_start__ = .;
    . += 1024 * 64;    

    /* stack */
    __stack_bottom__ = .;
    . += 1024 * 8;
    __stack_top__ = .;       

}
