.section ".text"

//Do a nonsense operation one cycle.
//Needed for when we want to do nothing for x number of cycles.
.global delay
delay:
    bx lr

