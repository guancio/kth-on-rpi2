
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_GUESTS 512


struct guest_data {
    const char *filename;    
    unsigned long vadr;
    int megs;
};


struct guest_data guests[MAX_GUESTS];
int guest_cnt;

/* ---------------------------------------------------------- */
int parse_hex(char *str, unsigned long *ret)
{
    sscanf(str, "0x%lx", ret); /* strtol wont work */
    return 1;    
}

int parse_int(char *str, int *ret)
{
    *ret = atol(str);    
    return 1;    
}

void parse_guests(int cnt, char **data)
{
    unsigned long tmp;
    int i, j;
    
    guest_cnt = 0;    
    for(i = 0; i < cnt; i++) {
        
        /* separate file and adr */
        char *file = data[i];
        
        char *padr = strchr(file, ':');
        if(!padr) {
            fprintf(stderr, "Invalid guest format: '%s'\n", file);
            exit(20);
        }
        *padr++ = '\0';
            
        char *vadr = strchr(padr, ':');
        if(!vadr) {
            fprintf(stderr, "Invalid guest format: '%s'\n", file);
            exit(20);
        }        
        *vadr++ = '\0';
                       
        /* save the extracted data */        
        if(!parse_hex(padr, & guests[guest_cnt].vadr) || 
           !parse_int(vadr, & guests[guest_cnt].megs))
            exit(20);
        guests[guest_cnt].filename = file;        
        guest_cnt++;
        
        if(guest_cnt >= MAX_GUESTS) {
            fprintf(stderr, "Too many guests!\n");
            exit(20);                    
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
           "\t.word 0xfa7ec0de  @@ guests magic\n"
           "\t.word %d        @@ guests count\n"
           "\t.word __guests_data_end - . @@ guests data end\n",
           guest_cnt);
    
    /* guest table */
    printf("__guests_table_start:\n");
    for(i = 0; i < guest_cnt; i++) {
        printf("\n"
               "@@ Guest #%d\n"
               "\t.word 2%03df - 1%03df @@ FWSIZE\n"            
               "\t.word 0x%08lx @@ PSIZE\n"
               "\t.word 0x%08lx @@ VADR\n",
               i + 1, i, i,               
               (unsigned long) guests[i].megs << 20,
               guests[i].vadr);
    }

    /* guest binary */
    printf("__guests_data_start:\n");
    for(i = 0; i < guest_cnt; i++) {
        printf(
                "1%03d:\t.incbin \"%s.bin\"\n"
                "\t.align 8\n"
                "2%03d:\n",
                i, guests[i].filename, i);
    }    
    printf("__guests_data_end:\n");

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
            
    case 'a':
        print_asm();
        break;
    default:
        fprintf(stderr, "Unknown command: %c\n", command);
        return 20;
    }
    return 0;
}
