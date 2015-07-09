
MEMORY
{
	ram : ORIGIN = 0x8000, LENGTH = 0x1000
}

SECTIONS
{
	. = 0x8000;
	.dummy : { *(.text.dummy*) } > ram
	.magic ALIGN(0x20): { *(.text.magic*) } > ram
	.startup : { *(.text.startup*) } > ram
	.functions : { *(.text.functions*) } > ram
	.bss : { *(.bss*) } > ram
}

