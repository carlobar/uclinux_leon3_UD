/*
 *  linux/arch/sparcnommu/kernel/setup.c
 *
 *  Copyleft  (C) 2000       James D. Schettine {james@telos-systems.com}
 *  Copyright (C) 1999-2002  Greg Ungerer (gerg@snapgear.com)
 *  Copyright (C) 1998,2000  D. Jeff Dionne <jeff@lineo.ca>
 *  Copyright (C) 1998       Kenneth Albanowski <kjahds@kjahds.com>
 *  Copyright (C) 1995       Hamish Macdonald
 */

/*
 * This file handles the architecture-dependent parts of system setup
 */

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/fb.h>
#include <linux/console.h>
#include <linux/genhd.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/major.h>
#include <linux/tty.h>

#include <asm/irq.h>

#ifdef CONFIG_BLK_DEV_INITRD
#include <linux/blk.h>
#include <asm/pgtable.h>
#endif

struct screen_info screen_info = {
	0, 0,			/* orig-x, orig-y */
	{ 0, 0, },		/* unused */
	0,			/* orig-video-page */
	0,			/* orig-video-mode */
	80,			/* orig-video-cols */
	0,0,0,			/* ega_ax, ega_bx, ega_cx */
	37,			/* orig-video-lines */
	0,                      /* orig-video-isVGA */
	16                      /* orig-video-points */
};


extern void register_console(void (*proc)(const char *));
extern void console_print_LEON(const char * b);

unsigned long rom_length;
unsigned long memory_start;
unsigned long memory_end;

char command_line[512];
char saved_command_line[512];
char console_device[16];

#ifdef CONFIG_SPARCNOMMU_SET_DEF_KERNCMD
char console_default[] = CONFIG_SPARCNOMMU_DEF_KERNCMD;
#else
char console_default[] = "CONSOLE=/dev/ttyS0";
#endif

#define CPU "LEON/Sparc2.1"


/*
 *	Setup the console. There may be a console defined in
 *	the command line arguments, so we look there first. If the
 *	command line arguments don't exist then we default to the
 *	first serial port. If we are going to use a console then
 *	setup its device name and the UART for it.
 */
void setup_console(void)
{
  //#ifdef CONFIG_COLDFIRE
/* This is a great idea.... Until we get support for it across all
   supported m68k platforms, we limit it to ColdFire... DJD */
	char *sp, *cp;
	int i;
	extern void rs_console_init(void);
	extern void rs_console_print(const char *b);
	extern int rs_console_setup(char *arg);

	/* Quickly check if any command line arguments for CONSOLE. */
	for (sp = NULL, i = 0; (i < sizeof(command_line)-8); i++) {
		if (command_line[i] == 0)
			break;
		if (command_line[i] == 'C') {
			if (!strncmp(&command_line[i], "CONSOLE=", 8)) {
				sp = &command_line[i + 8];
				break;
			}
		}
	}

	/* If no CONSOLE defined then default one */
	if (sp == NULL) {
		strcpy(command_line, console_default);
		sp = command_line + 8;
	}

	/* If console specified then copy it into name buffer */
	for (cp = sp, i = 0; (i < (sizeof(console_device) - 1)); i++) {
		if ((*sp == 0) || (*sp == ' ') || (*sp == ','))
			break;
		console_device[i] = *sp++;
	}
	console_device[i] = 0;

	/* If a serial console then init it */
	//if (rs_console_setup(cp)) {
	//	rs_console_init();
	//	register_console(rs_console_print);
	//}
	//#endif /* CONFIG_COLDFIRE */

#if defined(CONFIG_LEON_SERIAL) || defined(CONFIG_GRLIB_GAISLER_APBUART)
        
  register_console(console_print_LEON);
#endif

}

#if defined CONFIG_LEON_2  || defined CONFIG_LEON_3
#define CAT_ROMARRAY
unsigned int _ramend;
#endif

unsigned int bootloader_supplied_stack = 0;

void setup_arch(char **cmdline_p,
		unsigned long * memory_start_p, unsigned long * memory_end_p)
{
	extern int _stext, _etext;
	extern int _sdata, _edata;
	extern int _sbss, _ebss, _end;
	extern int _ramstart;
	unsigned int rend,i;

#ifdef CONFIG_LEON_3
	amba_init();
#endif

#if defined CONFIG_LEON_2 || defined CONFIG_LEON_3
	unsigned int memctrl1,memctrl2,num_bytes;
#else
	extern int _ramend;
#endif

#ifdef CAT_ROMARRAY
	extern int __data_rom_start;
	extern int __data_start;
	//	int *romarray = (int *)((int) &__data_rom_start +
	//			      (int)&_edata - (int)&__data_start);
extern char initrd_start[];
extern char initrd_end[];
#endif

 
#if defined CONFIG_LEON_2 || defined CONFIG_LEON_3

#define LEON_PREGS	0x80000000
#define LEON_MCFG1	0x00
#define LEON_MCFG2	0x04

#define LEON_MCFG2_SRAMDIS		0x00002000
#define LEON_MCFG2_SDRAMEN		0x00004000
#define LEON_MCFG2_SRAMBANKSZ		0x00001e00	/* [12-9] */
#define LEON_MCFG2_SRAMBANKSZ_SHIFT	9
#define LEON_MCFG2_SDRAMBANKSZ		0x03800000	/* [25-23] */
#define LEON_MCFG2_SDRAMBANKSZ_SHIFT	23

#ifdef CONFIG_LEON_2
	memctrl1 = *((unsigned long*)(LEON_PREGS+LEON_MCFG1));
	memctrl2 = *((unsigned long*)(LEON_PREGS+LEON_MCFG2));
#endif
        
	num_bytes = 0;

        if (!bootloader_supplied_stack) 
           while(1) { printk("bootloader_supplied_stack has to be set by startup code\n"); }

#ifdef CONFIG_LEON_2
        
        /* mem calculation is obsolete: mem size is bootloader_supplied_stack - 0x40000000
         * keep it anyway.
         */
	if (memctrl2 & LEON_MCFG2_SRAMDIS) {
          if (memctrl2 & LEON_MCFG2_SDRAMEN) {
            i = (memctrl2 & LEON_MCFG2_SDRAMBANKSZ)
              >> LEON_MCFG2_SDRAMBANKSZ_SHIFT;
            num_bytes = (4 << i)*1024*1024;
            
            {
              /* test for 2. sdram bank */
              unsigned long addr = 0x40000000+num_bytes;
              unsigned long old1 = *(unsigned long*)(addr);
              unsigned long old2 = *(unsigned long*)(addr+4);
              *(unsigned long*)(addr) = old1 + 1;
              *(unsigned long*)(addr+4) = old1 + 2;
              if ((old1+1) == *(unsigned long*)(addr)) {
                num_bytes = num_bytes + num_bytes;
              }
              *(unsigned long*)(addr) = old1;
              *(unsigned long*)(addr+4) = old2;
            }
              
          } else {
            while(1) { printk("Neather SRAM not SDRAM is enabled\n"); }
          }
          
	} else {
          i = (memctrl2 & LEON_MCFG2_SRAMBANKSZ)
            >> LEON_MCFG2_SRAMBANKSZ_SHIFT;
          num_bytes = (8 << i)*1024; /* starting from 8 k */
          
          if (memctrl2 & LEON_MCFG2_SDRAMEN) {
            printk("Note: Numa architecture not supported. Not useing sdram.\n");
          }
          {
              /* test for 2.-4. sdram bank */
            unsigned long sz = num_bytes;
            unsigned long j,addr = 0x40000000+sz;
            for (j = 0;j < 3;j++,addr = addr+sz) {
              unsigned long old1 = *(unsigned long*)(addr);
              unsigned long old2 = *(unsigned long*)(addr+4);
              *(unsigned long*)(addr) = old1 + 1;
              *(unsigned long*)(addr+4) = old1 + 2;
              if ((old1+1) == *(unsigned long*)(addr)) {
                num_bytes = num_bytes + sz;
              }
              *(unsigned long*)(addr) = old1;
              *(unsigned long*)(addr+4) = old2;
            }
          }
	}
#endif
        
        num_bytes = bootloader_supplied_stack - 0x40000000;

        printk("bootloader_supplied_stack: %x\n",bootloader_supplied_stack);
        
        _ramend = 0x40000000 + num_bytes;
        
        
	memory_start = &_end;
	memory_end = _ramend - 4096; /* <- stack area */
	rend = _ramend;
#else
	memory_start = &_end;
	memory_end = &_ramend - 4096; /* <- stack area */
	rend = &_ramend;
#endif


#if 0
	config_BSP(&command_line[0], sizeof(command_line));
#else
	command_line[0] = 0;
#endif
	setup_console();

#ifdef CONFIG_LEON_3
        //amba_prinf_config();
#endif
        
	printk("\x0F\r\n\nuClinux/Sparc\n");
	printk("Flat model support (C) 1998-2000 Kenneth Albanowski, D. Jeff Dionne\n");
	printk("LEON-2.1 Sparc V8 support (C) 2000 D. Jeff Dionne, Lineo Inc.\n");
	printk("LEON-2.2/LEON-2.3 Sparc V8 support (C) 2001 The LEOX team <team@leox.org>.\n");
#if 1 
	printk("KERNEL:TEXT=0x%x-0x%x DATA=0x%x-0x%x "
		"BSS=0x%x-0x%x\n", (int) &_stext, (int) &_etext,
		(int) &_sdata, (int) &_edata,
		(int) &_sbss, (int) &_ebss);

	extern int nr_free_pages;
	//printk("Memory available: %i (%x)\n",nr_free_pages,&nr_free_pages);

	printk("KERNEL -> ROMFS=0x%x-0x%x MEM=0x%x-0x%x "
		"STACK=0x%x-0x%x\n",
#ifdef CAT_ROMARRAY
	       (int) initrd_start, initrd_end,
#else
	       (int) &_ebss, (int) memory_start,
#endif
		(int) memory_start, (int) memory_end,
		(int) memory_end, (int) rend);
#endif
	init_task.mm->start_code = (unsigned long) &_stext;
	init_task.mm->end_code = (unsigned long) &_etext;
	init_task.mm->end_data = (unsigned long) &_edata;
	init_task.mm->brk = (unsigned long) &_end;

	ROOT_DEV = MKDEV(BLKMEM_MAJOR,0);

	printk("VFS: open root device %s\n",
	       kdevname(ROOT_DEV));

	/* Keep a copy of command line */
	*cmdline_p = &command_line[0];

	memcpy(saved_command_line, command_line, sizeof(saved_command_line));
	saved_command_line[sizeof(saved_command_line)-1] = 0;

#ifdef DEBUG
	if (strlen(*cmdline_p)) 
		printk("Command line: '%s'\n", *cmdline_p);
	else
		printk("No Command line passed\n");
#endif
	*memory_start_p = memory_start;
	*memory_end_p = memory_end;

#ifdef DEBUG
	printk("Done setup_arch\n");
#endif

}

asmlinkage int sys_ioperm(unsigned long from, unsigned long num, int on)
{
	return -EIO;
}

int get_cpuinfo(char * buffer)
{
    char *cpu, *mmu, *fpu;
    u_long clockfreq, clockfactor;

    cpu = CPU;
    mmu = "none";
    fpu = "none";
    clockfactor = 1;

    clockfreq = loops_per_sec*clockfactor;

    return(sprintf(buffer, "CPU:\t\t%s\n"
		   "MMU:\t\t%s\n"
		   "FPU:\t\t%s\n"
		   "Clocking:\t%lu.%1luMHz\n"
		   "BogoMips:\t%lu.%02lu\n"
		   "Calibration:\t%lu loops\n",
		   cpu, mmu, fpu,
		   clockfreq/100000,(clockfreq/100000)%10,
		   loops_per_sec/500000,(loops_per_sec/5000)%100,
		   loops_per_sec));

}

void arch_gettod(int *year, int *mon, int *day, int *hour,
		 int *min, int *sec)
{
		*year = *mon = *day = *hour = *min = *sec = 0;
}

