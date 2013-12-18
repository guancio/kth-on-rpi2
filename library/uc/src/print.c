// ---------------------------------------

#include <stdarg.h>

#include <uclib.h>

#ifdef COMPILE_HOST
#define printf _printf /* dont override the host functions */
 extern int putchar(int c);
 void printf_putchar(int c) { putchar(c); } /* direct all calls to stdio */

#else
 // these are provided by the hardware
 extern void printf_putchar(int c);
#endif

// ---------------------------------------

void printf_string(char *str)
{
    while(*str) 
        printf_putchar(*str++);
}



void printf_int(int i)
{
    int f = 0, neg = 0;
    char buffer[12];
    
    
    if(i < 0) {
        neg ++;
        i = - i;
    }
    do {
        buffer[f++] = '0' + (i % 10);
        i /= 10;
    } while(i);

    if(neg) buffer[f++] = '-';

    while(f) {
        printf_putchar( buffer[--f]);
    }
}

void printf_hex(uint32_t n, uint32_t size)
{
    int h;
    int i = 32 / 4 - 1;

    do {
        h = (n >> 28);
        n <<= 4;
        if(size ==2 && i >=2)
            		continue;
        if(h < 10) h += '0';
        else h += 'A' - 10;

        printf_putchar(h);
    }while(i--);
}

void printf_bin(uint32_t n)
{
    int i;
    for(i = 32; i != 0; i--) {
        if( (i != 32) && !(i & 3)) printf_putchar('_');
        printf_putchar( (n & 0x80000000) ? '1' : '0');
        n <<= 1;
    }
}

// -----------------------------------
void printf(const char *fmt, ...)
{
    int c;
    va_list args;

    va_start(args, fmt);

    
    for(;;) {
        c = *fmt;
        if(c == '\0') goto cleanup;
        
        fmt ++;
        if(c == '%') {
            c = *fmt;
            fmt++;
            
            // sanity check?
            if(c == '\0') {
                printf_putchar(c);
                goto cleanup;
            }
            
            switch(c) {
                
            case 'c':
                printf_putchar(va_arg(args, int));
                break;
                
            case 's':
                printf_string(va_arg(args, char *));
                break;
            case 'i':
            case 'd':
                printf_int(va_arg(args, int));
                break;
                
            case 'x':
                printf_hex(va_arg(args, uint32_t), 8);
                break;
            case '2':
                printf_hex(va_arg(args, uint32_t), 2);
                break;
                
            case 'b':
                printf_bin(va_arg(args, uint32_t));
                break;
            
            case '%':
                printf_putchar(c);
                break;
            default:
                printf_putchar('%');
                printf_putchar(c);
            }
            
        } else printf_putchar(c);
    }
    
    
    /* yes, gotos are evil. we know */
cleanup:
    va_end(args);    
}
