
MEMORY
{
	ram : ORIGIN = 0x1000000, LENGTH = 0x1000
}

SECTIONS
{
	. = 0x1000000;
	.dummy : { *(.text.dummy*) } > ram
	.magic ALIGN(0x20): { *(.text.magic*) } > ram
	.startup : { *(.text.startup*) } > ram
	.bss : { *(.bss*) } > ram
}

