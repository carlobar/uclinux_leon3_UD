#undef sparc
OUTPUT_FORMAT("elf32-sparc", "elf32-sparc", "elf32-sparc")
OUTPUT_ARCH(sparc)
ENTRY(_start)
SECTIONS
{
        . = 0x4000;
	.rom : {
	        LINUXIMAGE (.text)
        }
	
	. = 0x40000000;
	.data : {
	        LINUXIMAGE (.data)
        }
	
	.bss : {
	        LINUXIMAGE (.bss)
        }
	
	.rdimage : {
                . = ALIGN(0x1000);
                initrd_start = .;
                RDIMAGE 
                initrd_end = .;
        }

        /DISCARD/ : {
                *(.exit.text)
                *(.exit.data)
                *(.exitcall.exit)
                *(.comment)
        }
}
