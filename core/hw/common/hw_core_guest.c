#include <hw.h>


#include "guest_blob.h"


struct guests_database guests_db = { -1 /* needed to force it out of BSS */ }; 

struct guest_binary *get_guest(int index)
{
    struct guest_binary *guest;
    
    if(index < 0 || index >= guests_db.count) return NULL;    
    guest = & guests_db.guests[index];
    
    if(guest->size <= 0) return NULL;    
    return guest;
}
