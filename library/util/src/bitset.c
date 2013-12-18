
/*
 * a bitset
 */

#include <utillib.h>

bitset_t *bitset_init(int size)
{
    bitset_t *ret;
    
    /* make size a multiple of 32 */
    size  = (size + 31) & ~31;
    
    /* allocate it */
    ret = malloc(sizeof(bitset_t) + size / 8);
    if(ret) {
        ret->size = size;    
    }
    return ret;
}

void bitset_cleanup(bitset_t *b)
{
    if(b) free(b);
}

void bitset_set(bitset_t *b, int n, BOOL value)
{
    int index, bit;
    if(!b || n < 0 || n >= b->size) return;
    
    index = n >> 5;
    bit = 1UL << (n & 31);
    
    if(value) 
        b->data[index] |= bit;
    else
        b->data[index] &= ~bit;
}

BOOL bitset_get(bitset_t *b, int n)
{
    int index, bit;
    if(!b || n < 0 || n >= b->size) return FALSE;
    
    index = n >> 5;
    bit = 1UL << (n & 31);
    return b->data[index] & bit ? TRUE : FALSE;
}

void bitset_set_all(bitset_t *b, BOOL value)
{
    uint32_t val, cnt;
    uint32_t *ptr;
    if(!b) return;
    
    /* write to 32 bits at a time */
    ptr = & b->data[0];
    val = value ? -1 : 0;
    cnt = b->size >> 5; /* size in dwords */    
    
    while(cnt--) {
        *ptr++ = val;
    }
}
