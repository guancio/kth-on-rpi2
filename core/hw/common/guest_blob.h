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
	addr_t pstart;
	addr_t vstart;
	size_t psize;
	size_t fwsize;	
};

struct guests_database {
    uint32_t count;    
    uint32_t pstart, pend;
    struct guest_binary guests[MAX_GUESTS];

};

extern struct guests_database guests_db;
extern struct guest_binary *get_guest(int index);

#endif /* __ASSEMBLER__ */

#endif /* _GUEST_BLOB_H_ */
