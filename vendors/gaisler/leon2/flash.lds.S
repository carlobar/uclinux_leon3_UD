#undef sparc
#undef linux
OUTPUT_FORMAT("elf32-sparc", "elf32-sparc", "elf32-sparc")
OUTPUT_ARCH(sparc)
ENTRY(_start)
SECTIONS
{
	.stage1 0 : {
                _start = .;
                locore.o
        }

        . = 0x4000;
	.rom : {
		_stext = . ;
		text_start = . ;
		_romvec = . ;
        	LINUXIMAGE (.text)
		_etext = . ;
        }
	
	.rdimage : {
		. = ALIGN (4)	; 
		initrd_start = .;
                _initrd_start = .;
                RDIMAGE 
                initrd_end = .;
		. = ALIGN(0x10)	; 
		__data_rom_start = . ;
        }

	
	. = 0x40000000;
	.data : {
		_sdata = . ;
		__data_start = . ;
	        LINUXIMAGE (.data)
		. = ALIGN(0x10)	; 
		_edata = . ;
		edata = .; 
        }
	
	.bss : {
		. = ALIGN( 0x10 ) ; 
		_sbss =  .;
		_bss_start = .;
		__bss_start = .;
		__data_end = .;
	        LINUXIMAGE (.bss)
		*(COMMON)
		. = ALIGN( 0x10 ) ; 
		_ebss = . ;
		_bss_end = . ;
		__bss_end = . ;
		end = . ;
		_end = . ;
        }
	

        /DISCARD/ : {
                *(.exit.text)
                *(.exit.data)
                *(.exitcall.exit)
                *(.comment)
        }
}
