__page_size__ = 4096;

MEMORY 
{
    RAM : org = 0xFFFC0000, l = 256K
}

ENTRY(_start) 
 
SECTIONS
{
    . = 0xFFFC0000;

    __hyper_start__ = .;
    
    /* code (startup comes first) */    
    __hyper_code_start__ = .;
    .text : {
        *(.startup)
        *.o (.text)
        *.to(.text)        
    } > RAM
    
/*    . = ALIGN(__page_size__); */
    __hyper_code_end__ = .;

    
        
    /* DATA */
    __hyper_data_start__ = .;       
         
    .data : {
    __hyper_data_ro_start__ = .;    
        *.o (.rodata*)
        *.to (.rodata*)    
    __hyper_data_ro_end__ = .;    
        
    } > RAM
    
    .data ALIGN(__page_size__) : {
    __hyper_data_rw_end__ = .;        
        *.o (.data)
        *.to(.data)
    __hyper_data_rw_end__ = .;            
    } > RAM
    __hyper_data_end__ = .;


    /* BSS */
    __hyper_bss_start__ = .;
    .bss  : {
        *.o (.bss)
        *.to(.bss)
    } > RAM
    __hyper_bss_end__ = .;
        

            
    /* heap: up to 104K */    
    __hyper_heap_start__ = .;    
    . = 0 - 1024 * (64 + 32 + 8);
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
