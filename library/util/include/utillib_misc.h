
#ifndef _UTIL_MISC_H_
#define _UTIL_MISC_H_


/* a bit-set */
typedef struct {
    int size;
    uint32_t data[0];
} bitset_t;


extern bitset_t *bitset_init(int size);
extern void bitset_cleanup(bitset_t *);
extern void bitset_set(bitset_t *, int n, BOOL value);
extern BOOL bitset_get(bitset_t *, int n);
extern void bitset_set_all(bitset_t *, BOOL value);



#endif /* _UTIL_MISC_ */
