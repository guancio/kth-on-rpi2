#include <stdio.h>
#include <ctype.h>

#include <string.h>
#include <stdlib.h>
#include <signal.h>



/**********************************************************
 * parsing and IO
 **********************************************************/
int is_space(char c)
{
    return (c == ' ' || c == '\t' || c == '\r' || c == '\n');
}

int readLine(char *buffer, int size)
{
    int count;
    fgets(buffer, size-1, stdin);
        
    
    for(count = 0; count < size-1; count++) {
        if(buffer[count] == '\0' || buffer[count] == '\r' || buffer[count] == '\n')
            break;
    }
    buffer[count] = '\0';
    
    /* empty line ? */
    if(count == 0) return 0;
    
    return 1;
}

char *trimString(char *buffer) 
{
    char *a;
    
    if(!buffer) return 0;
    
    while(is_space(*buffer) && *buffer != '\0') buffer++;
    
    a = buffer;
    while(*a != '\0') a++;
    
    a--;
    while(is_space(*a) && a > buffer)
        *a-- = '\0';
    
    return *buffer == '\0' ? 0 : buffer;
}

char *getNextString(char *buffer) 
{
    if(buffer == 0) return 0;
    
    while(!is_space(*buffer) && *buffer != '\0') buffer++;
    if(*buffer == '\0') return 0;
    
    *buffer++ = '\0';
    while(is_space(*buffer)) buffer++;
    
    if(*buffer == '\0') return 0;
    return buffer;    
}

/* number parsing: since some ugly hacks are just to cute...
 * handle digits and hex startitg with 0x 
 */
int getNumberFromString(char *str, int *result)
{
    int n, ret, base, digits;
    if(str == 0) return 0;
    
    while( is_space( *str)) str++;
    if(*str == '\0') return 0; /* empty */
    
    if( str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        base = 16;        
        str += 2;
    } else {
        base = 10;
    }
    
    ret = 0;
    digits = 0;
    while( *str != '\0') {
        int c = *str++;
        if( is_space(c) && digits != 0) break; /* end of the number */
        
        if(c >= '0' && c <= '9') n = c - '0';
        else if( c >= 'A' && c <= 'F')n = c - 'A' + 10;
        else if( c >= 'a' && c <= 'f')n = c - 'a' + 10;
        else n = -1;
        
        if(n < 0 || n >= base) return 0; /* bad char */
        ret = ret * base + n;
        digits++;
    }
    
    if(digits == 0) return 0; /* nothing */
    
    *result = ret;
    return 1;
}


/***********************************************************
 * params
 ***********************************************************/

#define PARAM_COUNT 10
char * params[PARAM_COUNT];
int params_flags[256];
int params_count;

void params_set_line(char *line)
{
    char *tmp = line;
    int i, j;
    
    /* get parameters */
    for(params_count = 0;  params_count < PARAM_COUNT; ) {
        tmp = trimString(tmp);
        if(!tmp) break;
        params[params_count++] = tmp;
        tmp = getNextString(tmp);        
    }
    
    /* get flags */
    for(i = 0; i < 256; i++) params_flags[i] = 0;
    for(i = 0, j = 0; i < params_count; i++) {
        tmp = params[j] = params[i];
        
        if(tmp[0] == '-') {
            tmp++;
            while(*tmp) 
                params_flags[ *tmp++] = 0x10000000 | j; /* remember where we took it */
        } else {
            j++;
        }
    }
    params_count = j;    
}

int params_get_count() 
{ 
    return params_count; 
}
int params_has(int count)
{
    return params_count >= count;
}

const char *params_get(int index) 
{ 
    return index >= 0 && index < params_count ? params[index] : 0; 
}

int params_has_flag(char c)
{
    return params_flags[0xFF & c];
}

const char *params_next(const char *default_)
{
    int i;
    const char *ret;
    if(params_count < 1) return default_;
    
    ret = params[0];
    
    params_count--;
    for(i = 0; i < params_count; i++)
        params[i] = params[i+1];
    
    return ret;
}

