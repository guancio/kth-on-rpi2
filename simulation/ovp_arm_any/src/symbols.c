
/*
 * this file will load ELF symbols so we can enter variable and functions names
 * in the debugger instead of using their addresses
 */


#include <stdio.h>
#include <stdlib.h>

#define uint32_t unsigned int
#define uint16_t unsigned short
#define uint8_t unsigned char


typedef struct {
    unsigned char id[16];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint32_t entry;
    
    uint32_t phoff, shoff;
    uint32_t flags;
    
    uint16_t eh_size;
    uint16_t ph_size, ph_count;
    uint16_t sh_size, sh_count;
    uint16_t string_index;
} elf_header;


typedef struct {
    uint32_t type;
    uint32_t offset;
    uint32_t vaddr, paddr;
    uint32_t file_size, mem_size;
    uint32_t flags;
    uint32_t align;
} prog_header;


typedef struct {
    uint32_t name;
    uint32_t type;
    uint32_t flags;
    uint32_t addr;
    uint32_t offset, size;
    uint32_t link;
    uint32_t info, align, entsize;
} section_header;


typedef struct {
    uint32_t name, value, size;
    uint8_t info, other;
    uint16_t index;
} symbol_entry;



typedef struct symbol_ {
    uint32_t name;
    uint32_t addr;
    struct symbol_ *next;
} symbol_chain;

symbol_chain *symbols = 0;
char *strings = 0;


// #define IS_INTERNAL_LABEL(name) (name[0] == '$' || strstr(name, ".isra.1") != 0)
#define IS_INTERNAL_LABEL(name) (name[0] == '$')


/************************************************************************** 
 * 
 **************************************************************************/
void do_read_symbol_table(FILE *fp, uint32_t offset, uint32_t size, uint32_t entsize)
{
    symbol_entry symbol;
    int i, count;
    symbol_chain *s;
    
    if(entsize != sizeof(symbol)) {
        printf("Bad symbol entry size (%d vs %d)\n", entsize, sizeof(symbol));
        return;
    }
    
    count = 0;
    fseek(fp, offset, SEEK_SET);    
    for(i = 0; i < size / entsize; i++) {
        int bind, type;
        fread(&symbol, sizeof(symbol), 1, fp);
        
        bind = symbol.info >> 4;
        type = symbol.info & 0xF;
                
        
        /* only intrested in objects and functions */
        if( type != 0 && type != 1 & type != 2) continue;
        
        /* null name */
        if(symbol.name == 0) continue;
//        if(symbol.size == 0) continue;
        
//        printf("SYMBOL %d adr=%08lx size = %d, type=%d, bind=%d, idx=%d\n", symbol.name, symbol.value, symbol.size, type, bind, symbol.index); // DEBUG
        
        // allocate and put it on the list
        s = (symbol_chain *) malloc( sizeof(symbol_chain));
        s->next = symbols;
        s->name = symbol.name;
        s->addr = symbol.value;
        symbols = s;
        count++;
    }
    printf("SYMBOL: read %d symbols\n", count);
    
}

void do_read_string_table(FILE *fp, uint32_t offset, uint32_t size, uint32_t entsize)
{
    if(strings){
        free(strings);
        strings = 0;
    }
    
    strings = (char *) malloc(size);
    
    fseek(fp, offset, SEEK_SET);
    fread(strings, size, 1, fp);    
}


/************************************************************************** 
 * 
 **************************************************************************/
void doLoadSymbols(const char *filename)
{
    FILE *fp;
    elf_header header;
    prog_header prog;
    section_header section;
    int i;
    
    if(!filename) {
        printf("ERROR: no symbol file was given\n");
        return;
    }
    
    fp = fopen(filename, "rb");
    if(!fp) {
        printf("ERROR: could not open symbol file '%s'\n", filename);
        return;
    }
    
    fread(& header, sizeof(header), 1, fp);
    /*
    printf("ELF ID = %x%c%c%c\n", header.id[0], header.id[1], header.id[2], header.id[3]);
    printf("ELF type = %d, machine = %d, version = %d, entry = %08lx\n",
           header.type, header.machine, header.version, header.entry);
    
    printf("ELF %ph/sh-off=%08lx/%08lx, flags=%08lx\n", header.phoff, header.shoff, header.flags);
    printf("ELF: eh_size=%d, ph=%d/%d, sh=%d/%d, string_index=%d\n",
           header.eh_size,
           header.ph_size, header.ph_count,
           header.sh_size, header.sh_count,
           header.string_index
           );
       */
    if(header.id[0] == 0x7F && header.id[1] == 'E' && header.id[2] == 'L' && header.id[3] == 'F') {

        for(i = 0; i < header.sh_count; i++) {
            fseek(fp, header.shoff + header.sh_size * i, SEEK_SET);
            
            fread(&section, sizeof(section), 1, fp);
            switch(section.type) {
            case 2:
                do_read_symbol_table(fp, section.offset, section.size, section.entsize);
                break;
            case 3:
//                if(i == header.string_index) 
                    do_read_string_table(fp, section.offset, section.size, section.entsize);
                break;
                
            }
        }
    } else printf("ERROR: file is not an ELF\n");
    fclose(fp);
    
    if(strings == 0) {
        printf("ERROR: no string table was found\n");
        exit(0);
    }
}

int getAddressFromSymbol(char *str, int *result)
{
    symbol_chain *s = symbols;
    if(strings == 0) return 0;
    if(!str) return 0;
    
    while(s) {
        char *str2 = strings + s->name;
        if(!strcmp(str, str2)) {
            *result = s->addr;
            return 1;
        }
        s = s->next;
    }
    return 0;
}

char *getSymbolFromAddress(int addr)
{
    symbol_chain *s = symbols;
    
    if(strings == 0) return 0;
    
    while(s) {
        if(addr == s->addr) {
            char *ret = strings + s->name;
            if(!IS_INTERNAL_LABEL(ret))
                return ret;
        }
        s = s->next;
    }
    return 0;    
}

/* ------------------------------------------------------------------ */

int getClosestSymbolBefore(int adr, int *out_adr, char **out_label)
{
    return get_closest_symbol(adr, out_adr, out_label, 1);
}

int getClosestSymbolAfter(int adr, int *out_adr, char **out_label)
{
    return get_closest_symbol(adr, out_adr, out_label, 0);
}

int get_closest_symbol(int adr, int *out_adr, char **out_label, int before)
{

    int best = -1;
    symbol_chain *s = symbols;    
    
    if(strings == 0) return 0;
    
    if(!before) adr++; /* dont give us our own! */
    
    while(s) {
        unsigned int delta = s->addr - adr;
        
        if(before) delta = - delta;
        
        if(delta >= 0 && (best == -1 || delta < best)) {            
            char *name = strings + s->name;            
            if(!IS_INTERNAL_LABEL(name)) {
                best = delta;
                if(out_adr) *out_adr = s->addr;
                if(out_label)  *out_label = name;
            }        
        }
        s = s->next;
    }
    
    return best == -1 ? 0 : 1;          
}
