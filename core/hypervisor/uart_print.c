
#include <hw.h>

//Calls stdio_write_char, which is defined in the UART drivers.
void printf_putchar(char c)
{
    if(c=='\n')
        stdio_write_char('\r');
    stdio_write_char(c);
}
