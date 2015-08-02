__page_size__ = 4096;
/* _start = 0xF0001000; TODO: This should perhaps be defined explicitly here... */

MEMORY 
{
    VECTOR : org = 0xF0000000, l = 0x1000
    KUSER  : org = 0xF0000FA0, l = 0x1000 - 0xFA0
    RAM	   : org = 0xF0001000, l = 0x100000 - 0x1000 
}

ENTRY(_start) 
 
SECTIONS
{
    . = 0xF0000000;

    __hyper_start__ = . ;
    
    /* Vector table comes first, reserve 0x1000 for it */
    .vector : {
   		*.o(.vector) 
    } > VECTOR
    
    .kuser :{
    	*(.kuser)
    } > KUSER
       
    . =  0xF0001000;
    __hyper_code_start__ = .;
    __code_start__ = .;

    .text : {
    	*.o (.startup)
        *.o (.text)
        *.to(.text)        
    } > RAM
    
/*    . = ALIGN(__page_size__); */
    __hyper_code_end__ = .;
        
    /* Data */
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
    __data_end__ = .;
    __hyper_data_end__ = .;

    /* BSS */
    __hyper_bss_start__ = .;
    .bss  : {
        *.o (.bss)
        *.to(.bss)
    } > RAM
    __hyper_bss_end__ = .;
 
	/* TODO: Memory layout for the different cores... (below is temporary) */

    /* Heap: up to 104 KiB */    
    __hyper_heap_start__ = .;    
    . = 0xf0100000 - 1024 * (64 + 32 + 8);
    __hyper_heap_end__ = .;
    
    /* Primary stack (for core 0) */
    __hyper_stack_bottom__ = .;
    . += 1024 * 8;
    __hyper_stack_top__ = .;    

    /* Guest (running on core 0) */
    __hyper_guest_start__ = .;
    . += 1024 * 32;
    __hyper_guest_end__ = .;     
    
    /* Master PT memory */    
    . = ALIGN(__page_size__);
    __hyper_pt_start__ = .;
    . += 1024 * 64;    
    __hyper_pt_end__ = .;

	/* Secondary stack (for core 1) */
	__hyper_stack_bottom_core_1__ = .;
    . += 1024 * 8;
    __hyper_stack_top_core_1__ = .;

	/* Tertiary stack (for core 2) */
	__hyper_stack_bottom_core_2__ = .;
    . += 1024 * 8;
    __hyper_stack_top_core_2__ = .;

	/* Quaternary stack (for core 3) */
	__hyper_stack_bottom_core_3__ = .;
    . += 1024 * 8;
    __hyper_stack_top_core_3__ = .;    

	/* Secondary (slave) PT memory (only needed to run guests on different cores) */ 
    __hyper_pt_start_slave__ = .;
    . += 1024 * 64;    
    __hyper_pt_end_slave__ = .;

    __hyper_end__ = .;    
    
}
