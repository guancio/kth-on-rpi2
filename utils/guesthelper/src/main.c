
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_GUESTS 512

struct guest_data {
    const char *filename;
    unsigned long adr;
};


struct guest_data guests[MAX_GUESTS];
int guest_cnt;

/* ---------------------------------------------------------- */
void parse_guests(int cnt, char **data)
{
    unsigned long tmp;
    int i, j;
    
    guest_cnt = 0;    
    for(i = 0; i < cnt; i++) {
        
        /* separate file and adr */
        char *file = data[i];
        char *adr = strchr(file, ':');
        if(!adr) {
            fprintf(stderr, "Invalid guest format: '%s'\n", file);
            exit(20);
        }
        *adr++ = '\0';
        
        /* get the number */
        if(adr[0] != '0' || adr[1] != 'x') {
            fprintf(stderr, "Invalid number format: '%s'\n", adr);
            exit(20);            
        }
        adr += 2;
        tmp = strtol(adr, NULL, 16);
        
        /* save the extracted data */
        guests[guest_cnt].filename = file;
        guests[guest_cnt].adr = tmp;
        guest_cnt++;
        
        if(guest_cnt >= MAX_GUESTS) {
            fprintf(stderr, "Too many guests!\n");
            exit(20);                    
        }
    }
    
    /* sort after load address */
    for(j = 0; j < guest_cnt-1; j++) {
        int min = j;
        for(i = j + 1; i < guest_cnt; i++) {
            if(guests[i].adr > guests[min].adr) {
                min = i;
            }
        }
        if(min != j) {
            struct guest_data s = guests[j];
            guests[j] = guests[min];
            guests[min] = s;
        }            
    } 
}

/* ---------------------------------------------------------- */
void print_dependencies(const char *ext)
{
    int i;    
    for(i = 0; i < guest_cnt; i++) {
        printf("%s%s ", guests[i].filename, ext);
    }
    fflush(stdout);
}


void print_elf()
{
    print_dependencies(".elf");
}

void print_bin()
{
    print_dependencies(".bin");
}

/* ---------------------------------------------------------- */

void print_asm()
{
    int i;
    
    printf("\t.data\n"
           "\t.type guest_data, #object\n\n"
           "__guests_magic:\n\t.word 0xfa7ec0de\n\n"
           "__guests_data_size:\n\t.word __guests_data_end - __guests_data_start\n\n"
           "__guests_data_start:\n"
           );
    
          
    for(i = 0; i < guest_cnt; i++) {
        printf("\n"
               "@@ Guest #%d\n"
               "\t.word 2f - 1f @@ SIZE\n"
               "\t.word 0x%08lx @@ PADR\n"               
               "1:\t.incbin \"%s.bin\"\n"
               "2:\n",
               i + 1,
               guests[i].adr,               
               guests[i].filename);
    }
    
    printf("__guests_data_end:\n");

    fflush(stdout);
}

/* ---------------------------------------------------------- */
void print_positions()
{
    int i;
          
    for(i = 0; i < guest_cnt; i++) {
        printf("%s0x%08lx",
               i == 0 ? "" : ",",
               guests[i].adr
               );
    }
    fflush(stdout);
}

/* ---------------------------------------------------------- */
int main(int argc, char **argv)
{
    int i;
    char command;
    
    if(argc < 3) {
        fprintf(stderr, "Usage %s <command> <guests>\n", argv[0]);
        return 3;
    }
    
    command = argv[1][0];
    
    argv += 2;
    argc -= 2;
    parse_guests(argc, argv);
    
    switch(command) {
    case 'b': 
        print_bin();
        break;      
        
    case 'e': 
        print_elf();
        break;      
        
    case 'p': 
        print_positions();
        break;      
        
    case 'a':
        print_asm();
        break;
    default:
        fprintf(stderr, "Unknown command: %c\n", command);
        return 20;
    }
    return 0;
}
