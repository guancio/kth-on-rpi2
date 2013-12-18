
#ifndef _SYSTEMCALL_H_
#define _SYSTEMCALL_H_

struct svc0_data {
    uint32_t data[5];
    uint32_t reserved;
    uint32_t type;
};

extern uint32_t do_svc0(struct svc0_data *);


#endif /* _SYSTEMCALL_H_ */
