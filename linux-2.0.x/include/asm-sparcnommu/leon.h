/* control structure */
#ifndef _INCLUDE_LEON_H_
#define _INCLUDE_LEON_H_

#ifndef __ASSEMBLER__

struct lregs {
	volatile unsigned int memcfg1;		/* 0x00 */
	volatile unsigned int memcfg2;
	volatile unsigned int ectrl;
	volatile unsigned int failaddr;
	volatile unsigned int memstatus;		/* 0x10 */
	volatile unsigned int cachectrl;
	volatile unsigned int powerdown;
	volatile unsigned int writeprot1;
	volatile unsigned int writeprot2;	/* 0x20 */
	volatile unsigned int leonconf;
	volatile unsigned int dummy2;
	volatile unsigned int dummy3;
	volatile unsigned int dummy4;		/* 0x30 */
	volatile unsigned int dummy5;
	volatile unsigned int dummy6;
	volatile unsigned int dummy7;
	volatile unsigned int timercnt1;		/* 0x40 */
	volatile unsigned int timerload1;
	volatile unsigned int timerctrl1;
	volatile unsigned int wdog;
	volatile unsigned int timercnt2;   /* 0x50 */
	volatile unsigned int timerload2;
	volatile unsigned int timerctrl2;
	volatile unsigned int dummy8;
	volatile unsigned int scalercnt; /* 0x60 */
	volatile unsigned int scalerload;
	volatile unsigned int dummy9;
	volatile unsigned int dummy10;
	volatile unsigned int uartdata1; /* 0x70 */
	volatile unsigned int uartstatus1;
	volatile unsigned int uartctrl1;
	volatile unsigned int uartscaler1;
	volatile unsigned int uartdata2; /* 0x80 */
	volatile unsigned int uartstatus2;
	volatile unsigned int uartctrl2;
	volatile unsigned int uartscaler2;
	volatile unsigned int irqmask;    /* 0x90 */
	volatile unsigned int irqpend;
	volatile unsigned int irqforce;
	volatile unsigned int irqclear;
	volatile unsigned int piodata;
	volatile unsigned int piodir;
	volatile unsigned int pioirq;
};


#endif

/* control registers */

#define	PREGS	0x80000000
#define	MCFG1	0x00
#define	MCFG2	0x04
#define	ECTRL	0x08
#define	FADDR	0x0c
#define	MSTAT	0x10
#define CCTRL	0x14
#define PWDOWN	0x18
#define WPROT1	0x1C
#define WPROT2	0x20
#define LCONF 	0x24
#define	TCNT0	0x40
#define	TRLD0	0x44
#define	TCTRL0	0x48
#define	TCNT1	0x50
#define	TRLD1	0x54
#define	TCTRL1	0x58
#define	SCNT  	0x60
#define	SRLD  	0x64
#define	UDATA0 	0x70
#define	LEON_UDATA0 	0x70
#define	USTAT0 	0x74
#define	LEON_USTAT0 	0x74
#define	UCTRL0 	0x78
#define	USCAL0 	0x7c
#define	UDATA1 	0x80
#define	USTAT1 	0x84
#define	UCTRL1 	0x88
#define	USCAL1 	0x8c
#define	IMASK	0x90
#define	IPEND	0x94
#define	IFORCE	0x98
#define	ICLEAR	0x9c
#define	IOREG	0xA0
#define	IODIR	0xA4
#define	IOICONF	0xA8

/* ASI codes */
#define ASI_LEON_PCI		0x04
#define ASI_LEON_IFLUSH		0x05
#define ASI_LEON_DFLUSH		0x06
#define ASI_LEON_ITAG		0x0c
#define ASI_LEON_IDATA		0x0d
#define ASI_LEON_DTAG		0x0e
#define ASI_LEON_DDATA		0x0f
#define ASI_LEON_BYPASS		0x1c
#define ASI_LEON_FLUSH_PAGE	0x10
#define ASI_LEON_FLUSH_CTX	0x13
#define ASI_LEON_DCTX		0x14
#define ASI_LEON_ICTX		0x15
#define ASI_LEON_DCACHE_MISS	0x01

/* memory areas */

#define CRAM	0x40000000
#define IOAREA	0x20000000

/* Some bit field masks */

#define CCTRL_FLUSHING_MASK 0x0c000

#define CPP_CONF_BIT	28
#define CPP_CONF_MASK	3
#define FPU_CONF_BIT	4
#define FPU_CONF_MASK	3
#define CPTE_MASK	0x0c0000
#define ITE_BIT		16
#define IDE_BIT		14
#define DTE_BIT		12
#define DDE_BIT		10

#define UCTRL_RE	0x01
#define UCTRL_TE	0x02
#define UCTRL_RI	0x04
#define UCTRL_TI	0x08
#define UCTRL_PS	0x10
#define UCTRL_PE	0x20
#define UCTRL_FL	0x40
#define UCTRL_LB	0x80

#define USTAT_DR	0x01
#define USTAT_TS	0x02
#define USTAT_TH	0x04
#define USTAT_BR	0x08
#define USTAT_OV	0x10
#define USTAT_PE	0x20
#define USTAT_FE	0x40

#ifndef __ASSEMBLY__

static __inline__ void leon_store_reg(unsigned long paddr,unsigned long value)
{
	__asm__ __volatile__("st %0, [%1] \n\t": :
			     "r" (value), "r" (paddr),
			     "i" (0) : "memory");
}

static __inline__ unsigned long leon_load_reg(unsigned long paddr)
{
	unsigned long retval;
	__asm__ __volatile__("ld [%1] , %0\n\t" :
			     "=r" (retval) :
			     "r" (paddr), "i" (0));
	return retval;
}

#define LEON_REGLOAD_PA(x)		leon_load_reg ((unsigned long)(x)+PREGS)
#define LEON_REGSTORE_PA(x,v)		leon_store_reg((unsigned long)(x)+PREGS,(unsigned long)(v))

/* flush cache */
static __inline__ void leon2_dcache_flush(void) {
 	__asm__ __volatile__("sta %%g0, [%%g0] %0\n\t": :
			     "i" (ASI_LEON_DFLUSH) : "memory");
}
static __inline__ void leon2_icache_flush(void) {
	__asm__ __volatile__("sta %%g0, [%%g0] %0\n\t": :
			     "i" (ASI_LEON_IFLUSH) : "memory");
}
static __inline__ void leon2_cache_flush_all(void) {
    leon2_dcache_flush();
    leon2_icache_flush();
}
#endif

#ifdef CONFIG_LEON_3
#include <asm/leon3.h>
#endif

#endif

