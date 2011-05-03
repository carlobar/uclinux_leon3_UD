#undef sparc
#undef linux
OUTPUT_FORMAT("elf32-sparc", "elf32-sparc", "elf32-sparc")
OUTPUT_ARCH(sparc)
ENTRY(_start)
SECTIONS
{
        . = 0x40000000;
	.stage1 : {
		_start = .;
		start_gpio.o(start_gpio);
		locore_stage1.o
	}
        . = ALIGN(0x4000);
	.rom : {
        	_stext = . ;
		text_start = . ;
		_romvec = . ;
        	LINUXIMAGE (.text)
		_etext = . ;
        }
	
	.rdimage : {
		. = ALIGN (4)	; 
		__data_rom_start = . ; 
                initrd_start = .;
                _initrd_start = .;
                RDIMAGE 
                initrd_end = .;
        }
	
	.data : {
		_sdata = . ;
		__data_start = . ;
	        LINUXIMAGE (.data)
		_edata = . ;
		edata = ALIGN( 0x10 ) ;
        }
	
	.bss : {
		_sbss = ALIGN( 0x10 ) ;
		_bss_start = ALIGN( 0x10 ) ;
		__bss_start = ALIGN( 0x10 ) ;
		__data_end = ALIGN( 0x10 ) ;
	        LINUXIMAGE (.bss)
		*(COMMON)
		_ebss = . ;
		_bss_end = . ;
		__bss_end = . ;
		end = ALIGN( 0x10 ) ;
		_end = ALIGN( 0x10 ) ;
        }
	

        /DISCARD/ : {
                *(.exit.text)
                *(.exit.data)
                *(.exitcall.exit)
                *(.comment)
        }
}
