
MEMORY
{
	ram : ORIGIN = 0x8000, LENGTH = 0x1000
}

SECTIONS
{
	.startup : { *(.text.startup*) } > ram
	.magic : { *(.text.magic*) } > ram
	.bss : { *(.bss*) } > ram
}

