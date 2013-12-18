
#include <types.h>
#include <hw.h>
#include "mmu.h"

/* this should have been declared in board code */

extern memory_layout_entry * memory_padr_layout;

memory_layout_entry *mem_padr_lookup(uint32_t padr)
{
    memory_layout_entry * tmp = (memory_layout_entry*)(&memory_padr_layout);
    uint32_t page = ADDR_TO_PAGE(padr);
    
    
    for(;;) {
        if(!tmp) break;
                
        //if(page >= tmp->page_start && page <= tmp->page_start + tmp->page_count) return tmp;
        if(page >= (tmp->page_start & 0xFFF00) && page <= tmp->page_count) return tmp;
        if(tmp->flags & MLF_LAST) break;
        tmp++;
    }
    return 0; /* not found */
}
