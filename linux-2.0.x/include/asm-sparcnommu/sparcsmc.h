#ifndef __SPARC_SMC_H
#define __SPARC_SMC_H

#include <linux/config.h>

/*
 * swap functions are sometimes needed to interface little-endian hardware
 */
static inline unsigned short _swapw(volatile unsigned short v)
{
    return ((v << 8) | (v >> 8));
}

static inline unsigned int _swapl(volatile unsigned long v)
{
    return ((v << 24) | ((v & 0xff00) << 8) | ((v & 0xff0000) >> 8) | (v >> 24));
}

/* There is no difference between I/O and memory on 68k, these are the same */
#define inb(addr) \
    ({ unsigned char __v = (*(volatile unsigned char *) ((addr)));\
     __v; })
#define inw(addr) \
    ({ unsigned short __v = (*(volatile unsigned short *) (addr)); \
       _swapw(__v); })
#define inl(addr) \
    ({ unsigned int __v = (*(volatile unsigned int *) (addr)); _swapl(__v); })

#define outb(b,addr) ((*(volatile unsigned char *) ((addr))) = (b))
#define outw(b,addr) ((*(volatile unsigned short *) (addr)) = (_swapw(b)))
#define outl(b,addr) ((*(volatile unsigned int *) (addr)) = (_swapl(b)))

/* FIXME: these need to be optimized.  Watch out for byte swapping, they
 * are used mostly for Intel devices... */
#define outsw(addr,buf,len) \
    ({ unsigned short * __p = (unsigned short *)(buf); \
       unsigned short * __e = (unsigned short *)(__p) + (len); \
       while (__p < __e) { \
	  *(volatile unsigned short *)(addr) = *__p++;\
       } \
     })

#define insw(addr,buf,len) \
    ({ unsigned short * __p = (unsigned short *)(buf); \
       unsigned short * __e = (unsigned short *)(__p) + (len); \
       while (__p < __e) { \
          *(__p++) = *(volatile unsigned short *)(addr); \
       } \
     })

 
// Macros to replace x86 input and output macros.  these were taken from io.h
// in a coldfire ifdef but should work OK for the m68360 we are using here

static inline void outsl(unsigned int addr, void *buf, int len)
{
	volatile unsigned int *ap = (volatile unsigned int *) addr;
	unsigned int *bp = (unsigned int *) buf;
	while (len--)
		*ap = *bp++;
}

static inline void insb(unsigned int addr, void *buf, int len)
{
	volatile unsigned char *ap = (volatile unsigned char *) addr;
	unsigned char *bp = (unsigned char *) buf;
	while (len--)
		*bp++ = *ap;
}

static inline void insl(unsigned int addr, void *buf, int len)
{
	volatile unsigned int *ap = (volatile unsigned int *) addr;
	unsigned int *bp = (unsigned int *) buf;
	while (len--)
		*bp++ = *ap;
}

static inline void insl_align2(unsigned int addr, void *buf, int len)
{
	volatile unsigned int *ap = (volatile unsigned int *) addr;
	unsigned int *bp = (unsigned int *) buf;
	while (len--) {
          unsigned int  val = *ap;
          *((unsigned short *) bp)++ = (val >> 16) & 0xffff;
          *((unsigned short *) bp)++ = (val & 0xffff);
        }
        
}

typedef unsigned char    cyg_uint8  ;
typedef   signed char    cyg_int8   ;
typedef unsigned short   cyg_uint16 ;
typedef   signed short   cyg_int16  ;
typedef unsigned int     cyg_uint32 ;
typedef   signed int     cyg_int32  ;
typedef unsigned int     cyg_bool   ;

typedef cyg_uint32  CYG_WORD;
typedef cyg_uint8   CYG_BYTE;
typedef cyg_uint16  CYG_WORD16;
typedef cyg_uint32  CYG_WORD32;

#ifndef CYG_SWAP16
# define CYG_SWAP16(_x_)                                        \
    ({ cyg_uint16 _x = (_x_); ((_x << 8) | (_x >> 8)); })
#endif

#ifndef CYG_SWAP32
# define CYG_SWAP32(_x_)                        \
    ({ cyg_uint32 _x = (_x_);                   \
       ((_x << 24) |                            \
       ((0x0000FF00UL & _x) <<  8) |            \
       ((0x00FF0000UL & _x) >>  8) |            \
       (_x  >> 24)); })
#endif

# define CYG_CPU_TO_BE16(_x_) (_x_)
# define CYG_CPU_TO_BE32(_x_) (_x_)
# define CYG_BE16_TO_CPU(_x_) (_x_)
# define CYG_BE32_TO_CPU(_x_) (_x_)

# define CYG_CPU_TO_LE16(_x_) CYG_SWAP16((_x_))
# define CYG_CPU_TO_LE32(_x_) CYG_SWAP32((_x_))
# define CYG_LE16_TO_CPU(_x_) CYG_SWAP16((_x_))
# define CYG_LE32_TO_CPU(_x_) CYG_SWAP32((_x_))


/****************************************************************************/

#endif /* !__SPARC_SMC_H */
