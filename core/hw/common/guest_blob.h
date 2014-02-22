#ifndef _GUEST_BLOB_H_
#define _GUEST_BLOB_H_

/*
 * Guest "blob" definitions
 */

/* C and Assembler stuff */
#define GUESTS_MAGIC 0xfa7ec0de
#define MAX_GUESTS 8


/* C stuff */
#ifndef __ASSEMBLER__

struct guest_binary {
    uint32_t *ptr_phy;
    uint32_t *ptr_phy_tmp;
    uint32_t *ptr_va;
    uint32_t size;    
    
};

struct guests_database {
    uint32_t count;
    struct guest_binary guests[MAX_GUESTS];
};

extern struct guests_database guests_db;
extern struct guest_binary *get_guest(int index);

#endif /* __ASSEMBLER__ */

#endif /* _GUEST_BLOB_H_ */
