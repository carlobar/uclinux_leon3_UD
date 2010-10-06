#ifndef _INCLUDE_LEON_3_h_
#define _INCLUDE_LEON_3_h_

#include <asm/amba.h>

#ifndef __ASSEMBLER__

#define ASI_LEON3_CACHEMISS 1
#define ASI_LEON3_SYSCTRL   0x02
#define ASI_LEON3_DFLUSH    0x11

/* do a cache miss load */
static __inline__ unsigned long leon_load_reg_cachemiss(unsigned long paddr)
{
	unsigned long retval;
	__asm__ __volatile__("lda [%1] %2, %0\n\t":
			     "=r"(retval): "r"(paddr), "i"(ASI_LEON3_CACHEMISS));
	return retval;
}

/* do a cache miss load on byte access */
static inline unsigned char leon_load_reg_cachemiss8(unsigned long paddr)
{
	unsigned char retval;
	asm volatile("lduha [%1] %2, %0\n\t":
			     "=r"(retval): "r"(paddr), "i"(ASI_LEON3_CACHEMISS));
	return retval;
}

static __inline__ void leon_or_reg_cachemiss(unsigned long paddr,unsigned long value) {
  unsigned long v = leon_load_reg_cachemiss(paddr);
  *((unsigned long *)paddr) = v | value;
}

static __inline__ void leon_and_reg_cachemiss(unsigned long paddr,unsigned long value) {
  unsigned long v = leon_load_reg_cachemiss(paddr);
  *((unsigned long *)paddr) = v & value;
}

/* flush cache */
static __inline__ void leon3_dcache_flush(void) {
  __asm__ __volatile__("sta %%g0, [%%g0] %0\n\t": :
                       "i" (ASI_LEON3_DFLUSH) : "memory");
}
static __inline__ void leon3_icache_flush(void) {
   __asm__ __volatile__(" flush ");    
}
static __inline__ void leon3_cache_flush_all(void) {
    leon3_dcache_flush();
    leon3_icache_flush();
}

/*
 *  The following defines the bits in the LEON UART Status Registers.
 */

#define LEON_REG_UART_STATUS_DR   0x00000001 /* Data Ready */
#define LEON_REG_UART_STATUS_TSE  0x00000002 /* TX Send Register Empty */
#define LEON_REG_UART_STATUS_THE  0x00000004 /* TX Hold Register Empty */
#define LEON_REG_UART_STATUS_BR   0x00000008 /* Break Error */
#define LEON_REG_UART_STATUS_OE   0x00000010 /* RX Overrun Error */
#define LEON_REG_UART_STATUS_PE   0x00000020 /* RX Parity Error */
#define LEON_REG_UART_STATUS_FE   0x00000040 /* RX Framing Error */
#define LEON_REG_UART_STATUS_ERR  0x00000078 /* Error Mask */

 
/*
 *  The following defines the bits in the LEON UART Ctrl Registers.
 */

#define LEON_REG_UART_CTRL_RE     0x00000001 /* Receiver enable */
#define LEON_REG_UART_CTRL_TE     0x00000002 /* Transmitter enable */
#define LEON_REG_UART_CTRL_RI     0x00000004 /* Receiver interrupt enable */
#define LEON_REG_UART_CTRL_TI     0x00000008 /* Transmitter interrupt enable */
#define LEON_REG_UART_CTRL_PS     0x00000010 /* Parity select */
#define LEON_REG_UART_CTRL_PE     0x00000020 /* Parity enable */
#define LEON_REG_UART_CTRL_FL     0x00000040 /* Flow control enable */
#define LEON_REG_UART_CTRL_LB     0x00000080 /* Loop Back enable */

#define LEON3_GPTIMER_EN 1
#define LEON3_GPTIMER_RL 2
#define LEON3_GPTIMER_LD 4
#define LEON3_GPTIMER_IRQEN 8


/*
 *  The following defines the bits in the LEON PS/2 Status Registers.
 */

#define LEON_REG_PS2_STATUS_DR   0x00000001 /* Data Ready */
#define LEON_REG_PS2_STATUS_PE   0x00000002 /* Parity error */
#define LEON_REG_PS2_STATUS_FE   0x00000004 /* Framing error */
#define LEON_REG_PS2_STATUS_IH   0x00000008 /* Keyboard inhibit */


/*
 *  The following defines the bits in the LEON PS/2 Ctrl Registers.
 */

#define LEON_REG_PS2_CTRL_RE     0x00000001 /* Receiver enable */
#define LEON_REG_PS2_CTRL_TE     0x00000002 /* Transmitter enable */
#define LEON_REG_PS2_CTRL_KI     0x00000004 /* Keyboard interrupt enable */


typedef struct {
  volatile unsigned int ilevel;
  volatile unsigned int ipend;
  volatile unsigned int iforce;
  volatile unsigned int iclear;
  volatile unsigned int notused00;
  volatile unsigned int notused01;
  volatile unsigned int notused02;
  volatile unsigned int notused03;
  volatile unsigned int notused10;
  volatile unsigned int notused11;
  volatile unsigned int notused12;
  volatile unsigned int notused13;
  volatile unsigned int notused20;
  volatile unsigned int notused21;
  volatile unsigned int notused22;
  volatile unsigned int notused23;
  volatile unsigned int mask[16];
} LEON3_IrqCtrl_Regs_Map; 

typedef struct {
  volatile unsigned int data;
  volatile unsigned int status;
  volatile unsigned int ctrl;
  volatile unsigned int scaler;
} LEON3_APBUART_Regs_Map;


typedef struct {
  volatile unsigned int val;
  volatile unsigned int rld;
  volatile unsigned int ctrl;
  volatile unsigned int unused;
} LEON3_GpTimerElem_Regs_Map; 

typedef struct {
  volatile unsigned int scalar;
  volatile unsigned int scalar_reload;
  volatile unsigned int config;
  volatile unsigned int unused;
  volatile LEON3_GpTimerElem_Regs_Map e[8];
} LEON3_GpTimer_Regs_Map; 

typedef struct {
  volatile unsigned int iodata;
  volatile unsigned int ioout;
  volatile unsigned int iodir;
  volatile unsigned int irqmask;
  volatile unsigned int irqpol;
  volatile unsigned int irqedge;
} LEON3_IOPORT_Regs_Map;

typedef struct {
  volatile unsigned int write;
  volatile unsigned int dummy;
  volatile unsigned int txcolor;
  volatile unsigned int bgcolor;
} LEON3_VGA_Regs_Map;

typedef struct {
  volatile unsigned int data;
  volatile unsigned int status;
  volatile unsigned int ctrl;
} LEON3_APBPS2_REGS_Map;
extern volatile LEON3_APBPS2_REGS_Map *leon_apbps2;
/*
 *  Types and structure used for AMBA Plug & Play bus scanning 
 */

typedef struct amba_device_table {
  int            devnr;           /* numbrer of devices on AHB or APB bus */
  unsigned int   *addr[16];       /* addresses to the devices configuration tables */
  unsigned int   allocbits[1];       /* 0=unallocated, 1=allocated driver */
} amba_device_table;

typedef struct amba_confarea_type {
  amba_device_table ahbmst;
  amba_device_table ahbslv;
  amba_device_table apbslv;
  unsigned int      apbmst;
} amba_confarea_type;

extern unsigned long amba_find_apbslv_addr(unsigned long vendor, unsigned long device, unsigned long *irq);

typedef struct amba_apb_device {
  unsigned int   start, irq;
} amba_apb_device;

extern int amba_get_free_apbslv_devices (int vendor, int device,
                                         amba_apb_device *dev,int nr);

// collect ahb slaves
typedef struct amba_ahb_device {
	unsigned int start[4], irq;
} amba_ahb_device;
extern int amba_get_free_ahbslv_devices(int vendor, int device,
					amba_ahb_device * dev, int nr);


#define ASI_LEON3_SYSCTRL_ICFG		0x08
#define ASI_LEON3_SYSCTRL_DCFG		0x0c
#define ASI_LEON3_SYSCTRL_CFG_SNOOPING (1<<27)
#define ASI_LEON3_SYSCTRL_CFG_SSIZE(c) (1<<((c>>20)&0xf))

extern __inline__ unsigned long sparc_leon3_get_dcachecfg(void) {
	unsigned int retval;
	__asm__ __volatile__("lda [%1] %2, %0\n\t" :
			     "=r" (retval) :
			     "r" (ASI_LEON3_SYSCTRL_DCFG),
			     "i" (ASI_LEON3_SYSCTRL));
	return (retval);
}

extern __inline__ void sparc_leon3_enable_snooping(void) {
  //enable snooping
  __asm__ volatile ("lda [%%g0] 2, %%l1\n\t"  \
                    "set 0x800000, %%l2\n\t"  \
                    "or  %%l2, %%l1, %%l2\n\t" \
                    "sta %%l2, [%%g0] 2\n\t"  \
                    : : : "l1", "l2");	
};

extern __inline__ void sparc_leon3_disable_cache(void) {
  //asi 2
  __asm__ volatile ("lda [%%g0] 2, %%l1\n\t"  \
                    "set 0x00000f, %%l2\n\t"  \
                    "andn  %%l2, %%l1, %%l2\n\t" \
                    "sta %%l2, [%%g0] 2\n\t"  \
                    : : : "l1", "l2");	
};


#endif //!__ASSEMBLER__




#endif 

