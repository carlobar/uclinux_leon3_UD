/* Ethernet driver for Gaisler Research's GRETH Ethernet MAC (www.gaisler.com)
 * 
 * Based on:
 * Ethernet driver for Opencores ethernet controller (www.opencores.org).
 *      Copyright (c) 2002 Simon Srot (simons@opencores.org)
 *
 */


#include <linux/autoconf.h>
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/ptrace.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <asm/irq.h>
#include <asm/pgtable.h>
#include <asm/bitops.h>

#include <asm/leon.h>

#include "greth.h"

#define net_device device
#define __pa(x) (x)
#define __va(x) (x)

#define FLUSH_CACHE 1

#define _print printk

#define _FUNCDBG(a)
/*#define _FUNCDBG(a) printk(a) */
#define DEBUG
/* #define DEBUG_TX_PACKETS */
/* #define DEBUG_RX_PACKETS */

#define NET_IP_ALIGN    2
 
#define MACADDR0 CONFIG_GRLIB_GAISLER_MACADDR0
#define MACADDR1 CONFIG_GRLIB_GAISLER_MACADDR1
#define MACADDR2 CONFIG_GRLIB_GAISLER_MACADDR2
#define MACADDR3 CONFIG_GRLIB_GAISLER_MACADDR3
#define MACADDR4 CONFIG_GRLIB_GAISLER_MACADDR4
#define MACADDR5 CONFIG_GRLIB_GAISLER_MACADDR5

/* How many buffers per page 
 */
#define GRETH_RX_BUF_PPGAE	(PAGE_SIZE/GRETH_RX_BUF_SIZE)
#define GRETH_TX_BUF_PPGAE	(PAGE_SIZE/GRETH_TX_BUF_SIZE)

/* How many pages are needed for buffers
 */
#define GRETH_RX_BUF_PAGE_NUM	(GRETH_RXBD_NUM/GRETH_RX_BUF_PPGAE)
#define GRETH_TX_BUF_PAGE_NUM	(GRETH_TXBD_NUM/GRETH_TX_BUF_PPGAE)

/* Buffer size, Gbit MAC uses the tagged maximum frame size which 
   is 1518 excluding CRC   */
#define MAX_FRAME_SIZE		1518 

/* The buffer descriptors track the ring buffers.   
 */
struct greth_private {
   struct sk_buff* rx_skbuff[GRETH_RXBD_NUM];
   struct sk_buff* tx_skbuff[GRETH_TXBD_NUM];
   
   u16 tx_next;			
   u16 tx_last;			
   u16 tx_free;			
   u16 rx_cur;
   
   u8 phyaddr;
   u8 sp;
   u8 gb;
   u8 fd;
   u8 gbit_mac;
   u8 auto_neg;
   
   greth_regs *regs;			/* Address of controller registers. */
   greth_bd   *rx_bd_base;		/* Address of Rx BDs. */
   greth_bd   *tx_bd_base;		/* Address of Tx BDs. */
   
   int irq;
   struct enet_statistics stats;
};

static int greth_open(struct net_device *dev);
static int greth_start_xmit(struct sk_buff *skb, struct net_device *dev);
static void greth_rx(struct net_device *dev);
static void greth_rx_gbit(struct net_device *dev);
static void greth_tx(struct net_device *dev);
static void greth_interrupt(int irq, void *dev_id, struct pt_regs *regs);
static int greth_close(struct net_device *dev);
static struct enet_statistics *greth_get_stats(struct net_device *dev);
static int greth_set_mac_add(struct net_device *dev, void *p);
/*static void greth_set_multicast_list(struct net_device *dev);*/

#define GRETH_REGLOAD(a)	        ( leon_load_reg_cachemiss(&(a)) )
#define GRETH_REGSAVE(a,v)              (a=v)
#define GRETH_REGORIN(a,v)              (GRETH_REGSAVE(a,(GRETH_REGLOAD(a) | (v))))
#define GRETH_REGANDIN(a,v)             (GRETH_REGSAVE(a,(GRETH_REGLOAD(a) & (v))))

#ifdef DEBUG
static void
greth_print_packet(unsigned long add, int len)
{
	int i;

	_print("ipacket: add = %x len = %d\n", add, len);
	for(i = 0; i < len; i++) {
  		if(!(i % 16))
    			_print("\n");
  		_print(" %.2x", *(((unsigned char *)add) + i));
	}
	_print("\n");
}


#endif

/* Wait for a register change with a timeout, jiffies used has time reference */
#define wait_loop(wait_statement,timeout_in_secs,goto_label_on_timeout,arg_on_timeout) \
	{ \
		unsigned long _timeout = jiffies + HZ*timeout_in_secs; \
		while( wait_statement ){ \
			if ( ((long)_timeout - (long)jiffies) < 0 ){ \
				arg_on_timeout; \
				goto goto_label_on_timeout; \
			} \
		} \
	}

/* Return Error:
 *  0: no error
 *  -1: invalid input
 *  -2: MII busy (4s timeout)
 *  -3: MII busy after write to PHY (4s timeout)
 */
static int read_mii(int phyaddr, int regaddr, volatile greth_regs *regs)
{
	int err = 0;

	wait_loop( (GRETH_REGLOAD(regs->mdio) & GRETH_MII_BUSY), \
	           4, \
	           timeout, \
	           err=-2);
						 
	GRETH_REGSAVE(regs->mdio, ((phyaddr & 0x1F) << 11) | ((regaddr & 0x1F) << 6) | 2); 
        
	wait_loop( (GRETH_REGLOAD(regs->mdio) & GRETH_MII_BUSY), \
	           4, \
	           timeout, \
	           err=-3);
        
  if (!(GRETH_REGLOAD(regs->mdio) & GRETH_MII_NVALID)) {
		return (GRETH_REGLOAD(regs->mdio) >> 16) & 0xFFFF;
	}else {
		return -1;
  }
timeout:
	return err; 
}

/* Return Error:
 *  0: no error
 *  -2: MII busy (4s timeout)
 *  -3: MII busy after write to PHY (4s timeout)
 */
static int write_mii(int phyaddr, int regaddr, int data, volatile greth_regs *regs)
{
	int err = 0;
	
	wait_loop( (GRETH_REGLOAD(regs->mdio) & GRETH_MII_BUSY), \
	           4, \
	           timeout, \
	           err=-2);
        
	GRETH_REGSAVE(regs->mdio, ((data&0xFFFF)<<16) | ((phyaddr & 0x1F) << 11) | ((regaddr & 0x1F) << 6) | 1);
        
	wait_loop( (GRETH_REGLOAD(regs->mdio) & GRETH_MII_BUSY), \
	           4, \
	           timeout, \
	           err=-3);
						 				
	return 0;
timeout:
	return err;
}

static int
greth_open(struct net_device *dev)
{
	struct greth_private *cep = (struct greth_private *) dev->priv;
        struct sk_buff *skb;
	greth_regs *regs = (greth_regs *)dev->base_addr;

	_FUNCDBG("greth_open\n");
	
        volatile greth_bd *rx_bd;
	int i;

        rx_bd = cep->rx_bd_base;
        
        if (cep->gbit_mac) {
                for(i = 0; i < GRETH_RXBD_NUM; i++) {
                        skb = dev_alloc_skb(MAX_FRAME_SIZE);
                        skb_reserve(skb, NET_IP_ALIGN);
                        if (skb == NULL) {
                                break;
                        }
                        rx_bd[i].stat = GRETH_BD_EN | GRETH_BD_IE;
                        rx_bd[i].addr = (uint)__pa(skb->data);
                        cep->rx_skbuff[i] = skb;
                }
        } else {
                for(i = 0; i < GRETH_RXBD_NUM; i++) {
                        rx_bd[i].stat = GRETH_BD_EN | GRETH_BD_IE;
                }
        }
        GRETH_REGORIN(rx_bd[GRETH_RXBD_NUM - 1].stat, GRETH_BD_WR);
        
        /* Install our interrupt handler.
	 */
	request_irq(cep->irq, greth_interrupt, 0, "eth", (void *)dev);

	/* Enable receiver
	 */
	GRETH_REGORIN(regs->control, GRETH_RXEN | GRETH_RXI | GRETH_TXI);
        return 0;
        
}

static int
greth_close(struct net_device *dev)
{
	struct greth_private *cep = (struct greth_private *)dev->priv;
	greth_regs *regs = (greth_regs *)dev->base_addr;
	volatile greth_bd *bdp;
	int i;

	_FUNCDBG("greth_close\n");
	
	/* Free interrupt hadler 
	 */
	free_irq(cep->irq, (void *)dev);

	/* Disable receiver and transmitesr 
	 */
	GRETH_REGANDIN(regs->control, ~(GRETH_RXEN | GRETH_TXEN));

	bdp = cep->rx_bd_base;
	for (i = 0; i < GRETH_RXBD_NUM; i++) {
	        GRETH_REGANDIN(bdp->stat,~(GRETH_RXBD_STATUS | GRETH_BD_EN));
		bdp++;
	}

	bdp = cep->tx_bd_base;
	for (i = 0; i < GRETH_TXBD_NUM; i++) {
	        GRETH_REGANDIN(bdp->stat, ~(GRETH_RXBD_STATUS | GRETH_BD_EN));
		bdp++;
	}
        
        if (cep->gbit_mac) {
                /* Free all allocated rx buffers */
                for (i = 0; i < GRETH_RXBD_NUM; i++) {
                        if (cep->rx_skbuff[i] != NULL)
                                dev_kfree_skb(cep->rx_skbuff[i], FREE_READ);
                }

                /* Free all allocated tx buffers */
                for (i = 0; i < GRETH_TXBD_NUM; i++) {
                        if (cep->tx_skbuff[i] != NULL)
                                dev_kfree_skb(cep->tx_skbuff[i], FREE_WRITE);
                }
        }
        return 0;
}

static int
greth_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct greth_private *cep = (struct greth_private *)dev->priv;
	greth_regs *regs;
	volatile greth_bd *bdp;
	unsigned long flags, err=0;
        
        _FUNCDBG("greth_start_xmit\n");
	
        save_flags(flags);
        cli();

	regs = (greth_regs *)dev->base_addr;
	
	/* Fill in a Tx ring entry 
	 */
	bdp = cep->tx_bd_base + cep->tx_next;

        if (cep->tx_free <= 0) {
                /* All transmit buffers are full.  Stop queue
		 */
                dev->tbusy = 1;
                err = 1;
                goto out;
	}

#ifdef DEBUG_TX_PACKETS
	_print("TX\n");
	greth_print_packet(skb->data, skb->len);
#endif

        /* Copy data in preallocated buffer */
	if (skb->len > MAX_FRAME_SIZE) {
                err = 1;
                goto out;
	} else {
                memcpy((unsigned char *)__va(GRETH_REGLOAD(bdp->addr)), skb->data, skb->len); 
	}
        bdp->stat =  GRETH_BD_EN | GRETH_BD_IE | (skb->len & GRETH_BD_LEN) | \
                    ((cep->tx_next == GRETH_TXBD_NUM_MASK) ? GRETH_BD_WR : 0);
	GRETH_REGORIN(regs->control, GRETH_TXEN);
        
        dev_kfree_skb(skb, FREE_WRITE);

        cep->tx_next = ((cep->tx_next + 1) & GRETH_TXBD_NUM_MASK);

        /* need to disable interrupts since free can be changed in interrupt handler*/
        cep->tx_free = cep->tx_free - 1;
        
        /* Send it on its way.  Tell controller its ready, interrupt when done,
	 * and to put the CRC on the end.
	 */
        dev->trans_start = jiffies;
out:
        
        restore_flags(flags);

        return err;
}

static void
greth_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	struct	net_device *dev = dev_id;
	volatile struct	greth_private *cep;
	uint	int_events;

	_FUNCDBG("greth_interrupt\n");
	
	cep = (struct greth_private *)dev->priv;

	/* Get the interrupt events that caused us to be here.
	 */
	int_events = GRETH_REGLOAD(cep->regs->status);
	GRETH_REGORIN(cep->regs->status, int_events & (GRETH_INT_RX | GRETH_INT_TX) );

	/* Handle receive event in its own function.
	 */
	if (int_events & GRETH_INT_RX) {
                if (cep->gbit_mac) {
                        greth_rx_gbit(dev_id);
                } else {
                        greth_rx(dev_id);
                }
        }
        
        /* Handle transmit event in its own function.
	 */
	if (int_events & GRETH_INT_TX) {
              
                greth_tx(dev_id);
                dev->tbusy = 0;
                mark_bh(NET_BH);
        }

	return;
}


static void
greth_tx(struct net_device *dev)
{
	struct	greth_private *cep;
	volatile greth_bd *bdp;
	unsigned int stat;
        
        _FUNCDBG("greth_tx\n");
	
	cep = (struct greth_private *)dev->priv;

	for (;;) {
	  	bdp = cep->tx_bd_base + cep->tx_last;
		stat = GRETH_REGLOAD(bdp->stat);
	
		if (stat & GRETH_BD_EN)
			break;

                if (cep->tx_free == GRETH_TXBD_NUM) 
                        break;
                
                /* Check status for errors
		 */
		if (stat & GRETH_TXBD_STATUS) {
                        if (stat & GRETH_TXBD_ERR_AL)
                                cep->stats.tx_aborted_errors++;
                        if (stat & GRETH_TXBD_ERR_UE)
                                cep->stats.tx_fifo_errors++;
                }
                cep->stats.tx_packets++;
                cep->tx_last = (cep->tx_last + 1) & GRETH_TXBD_NUM_MASK;
                cep->tx_free++;
        }
}

static void
greth_rx(struct net_device *dev)
{
	struct	greth_private *cep;
	greth_regs *regs = (greth_regs *)dev->base_addr;
	volatile greth_bd *bdp;
	struct	sk_buff *skb;
	int	pkt_len;
	int	bad;
	unsigned int stat;
        
        _FUNCDBG("greth_rx\n");
	
	cep = (struct greth_private *)dev->priv;

	/* First, grab all of the stats for the incoming packet.
	 * These get messed up if we get called due to a busy condition.
	 */
	for (;;cep->rx_cur = (cep->rx_cur + 1) & GRETH_RXBD_NUM_MASK) {

		bdp = cep->rx_bd_base + cep->rx_cur;
		stat = GRETH_REGLOAD(bdp->stat);
		bad = 0;

                if (stat & GRETH_BD_EN)
			break;
			
		/* Check status for errors.
		 */
		if (stat & GRETH_RXBD_ERR_FT) {
			cep->stats.rx_length_errors++;
                        bad = 1;
		}
		if (stat & (GRETH_RXBD_ERR_AE | GRETH_RXBD_ERR_OE)) {
			cep->stats.rx_frame_errors++;
                        bad = 1;
		}
		if (stat & GRETH_RXBD_ERR_CRC) {
			cep->stats.rx_crc_errors++;
                        bad = 1;
		}
		if (bad) {
                        cep->stats.rx_errors++;
                        bdp->stat = GRETH_BD_EN | GRETH_BD_IE | ((cep->rx_cur == GRETH_RXBD_NUM_MASK) ? GRETH_BD_WR : 0);
                        GRETH_REGORIN(regs->control, GRETH_RXEN);
                        continue;
		}

		/* Process the incoming frame.
		 */
		pkt_len = stat & GRETH_BD_LEN;
                
                skb = dev_alloc_skb(pkt_len+NET_IP_ALIGN);
                
                if (skb == NULL) {
			printk("%s: Memory squeeze, dropping packet.\n", dev->name);
			cep->stats.rx_dropped++;
		} else {
			skb_reserve(skb, NET_IP_ALIGN);
			skb->dev = dev;
			
#ifdef DEBUG_RX_PACKETS
			_print("RX\n");
			greth_print_packet(__va(GRETH_REGLOAD(bdp->addr)), pkt_len);
#endif 

#ifdef FLUSH_CACHE
                        leon3_dcache_flush();
#endif
                        memcpy(skb_put(skb, pkt_len), (unsigned char *)__va(GRETH_REGLOAD(bdp->addr)), pkt_len);
                        skb->protocol = eth_type_trans(skb, dev);
			netif_rx(skb);
			cep->stats.rx_packets++;
		}
                bdp->stat = GRETH_BD_EN | GRETH_BD_IE | ((cep->rx_cur == GRETH_RXBD_NUM_MASK) ? GRETH_BD_WR : 0); 
		GRETH_REGORIN(regs->control, GRETH_RXEN);
        }
}

static void
greth_rx_gbit(struct net_device *dev)
{
	struct	greth_private *cep;
	greth_regs *regs = (greth_regs *)dev->base_addr;
	volatile greth_bd *bdp;
	struct	sk_buff *skb;
	int	pkt_len;
	int	bad;
	unsigned int stat;
        
        _FUNCDBG("greth_rx_gbit\n");
	
	cep = (struct greth_private *)dev->priv;

	/* First, grab all of the stats for the incoming packet.
	 * These get messed up if we get called due to a busy condition.
	 */
	for (;;cep->rx_cur = (cep->rx_cur + 1) & GRETH_RXBD_NUM_MASK) {

		bdp = cep->rx_bd_base + cep->rx_cur;
		stat = GRETH_REGLOAD(bdp->stat);
		bad = 0;

                skb = cep->rx_skbuff[cep->rx_cur];

                if (stat & GRETH_BD_EN)
			break;

                /* Check status for errors.
		 */
		if (stat & GRETH_RXBD_ERR_FT) {
			cep->stats.rx_length_errors++;
			bad = 1;
                } else if (stat & (GRETH_RXBD_ERR_AE | GRETH_RXBD_ERR_OE | GRETH_RXBD_ERR_LE)) {
			cep->stats.rx_frame_errors++;
			bad = 1;
                } else if (stat & GRETH_RXBD_ERR_CRC) {
			cep->stats.rx_crc_errors++;
			bad = 1;
                } 

                if (bad) {
                        cep->stats.rx_dropped++;
                        bdp->stat = GRETH_BD_EN | GRETH_BD_IE | ((cep->rx_cur == GRETH_RXBD_NUM_MASK) ? GRETH_BD_WR : 0);
                        GRETH_REGORIN(regs->control, GRETH_RXEN);
                        continue;
		}

                if(((stat & GRETH_RXBD_UDP_DEC) && !(stat & (GRETH_RXBD_UDP_CSERR | GRETH_RXBD_IP_CSERR))) || 
                   ((stat & GRETH_RXBD_TCP_DEC) && !(stat & (GRETH_RXBD_TCP_CSERR | GRETH_RXBD_IP_CSERR)))) {
                        skb->ip_summed = CHECKSUM_UNNECESSARY;
                } else {
                        skb->ip_summed = CHECKSUM_NONE;
                }
                
                /* Process the incoming frame.
		 */
                pkt_len = stat & GRETH_BD_LEN;

                skb->dev = dev;
                skb_put(skb, pkt_len);
                skb->protocol = eth_type_trans(skb, dev);
                netif_rx(skb);
                cep->stats.rx_packets++;
#ifdef DEBUG_RX_PACKETS
                _print("RX packet\n");
                greth_print_packet(__va(bdp->addr), pkt_len);
#endif
                skb = dev_alloc_skb(MAX_FRAME_SIZE);
                skb_reserve(skb, NET_IP_ALIGN);
                
                if (skb) {
                        cep->rx_skbuff[cep->rx_cur] = skb;
                        bdp->addr = (uint)__pa(skb->data);
                        bdp->stat = GRETH_BD_EN | GRETH_BD_IE | ((cep->rx_cur == GRETH_RXBD_NUM_MASK) ? GRETH_BD_WR : 0);
                        GRETH_REGORIN(regs->control, GRETH_RXEN);
                } else {
                        cep->rx_skbuff[cep->rx_cur] = NULL;	
                }
        }
        
}


static struct enet_statistics *greth_get_stats(struct net_device *dev)
{
        struct greth_private *cep = (struct greth_private *)dev->priv;
 
        return &cep->stats;
}

static int greth_set_mac_add(struct net_device *dev, void *p)
{
	struct sockaddr *addr=p;
	volatile greth_regs *regs;

	memcpy(dev->dev_addr, addr->sa_data,dev->addr_len);

	regs = (greth_regs *)dev->base_addr;

	GRETH_REGSAVE(regs->esa_msb, 	
                                addr->sa_data[0] << 8 	| 
				addr->sa_data[1]);
        GRETH_REGSAVE(regs->esa_lsb,
                          	addr->sa_data[2] << 24 	| 
				addr->sa_data[3] << 16 	| 
				addr->sa_data[4] << 8 	| 
				addr->sa_data[5]);
	return 0;
}
/*
static void greth_set_multicast_list(struct net_device *dev) {
  ;
}
*/

#define write_mii_(phyaddr,regaddr,data,regs,goto_label_on_timeout,timeout_arg,store_error) \
	{ \
		int _ret; \
		_ret = write_mii(phyaddr,regaddr,data,regs); \
		if ( _ret < 0) { \
			store_error = _ret;\
			timeout_arg; \
			goto goto_label_on_timeout; \
		}\
	}

#define wait_read_mii(mask,phyaddr,regaddr,regs,timeout_in_secs,goto_label_on_timeout,timeout_arg,store_error) \
	{ \
		unsigned long _timeout = jiffies + HZ*timeout_in_secs; \
		while( ((tmp=read_mii(phyaddr,regaddr,regs))>=0) && (tmp & mask) ) { \
			if ( ((long)_timeout - (long)jiffies) < 0) { \
				store_error = -10; \
				timeout_arg; \
				goto goto_label_on_timeout; \
			} \
		} \
		if ( tmp < 0 ) { \
			store_error = tmp;\
			timeout_arg; \
			goto goto_label_on_timeout; \
		} \
	}
	
#define read_mii_(result,phyaddr,regaddr,regs,goto_label_on_timeout,timeout_arg,store_error) \
	{ \
		result = read_mii(phyaddr,regaddr,regs); \
		if ( result < 0 ) { \
			store_error = result;\
			timeout_arg; \
			goto goto_label_on_timeout; \
		} \
	}

/* Initialize the GRETH MAC.
 */
int gaisler_greth_probe(struct device *dev)
{
        struct greth_private *cep;
	volatile greth_regs *regs;
	volatile greth_bd *tx_bd, *rx_bd;
	int i, j, k, tmp;
        int tmp1, tmp2;
        unsigned long mem_addr;
        int err;
				int timeout_error = 0, timeout_error_desc = 0;
				char *timeout_error_str[] = {
					/* 0 */ "",
					/* 1 */  "GRETH: Timeout waiting for GRETH core reset\n",
					
					/* 2 */  "GRETH: Timeout waiting for GRETH MII write 1 %d\n",
					/* 3 */  "GRETH: Timeout waiting for GRETH MII write 2 %d\n",
					/* 4 */  "GRETH: Timeout waiting for GRETH MII write 3 %d\n",
					
					/* 5 */  "GRETH: Timeout waiting for GRETH MII read 1 %d\n",
					/* 6 */  "GRETH: Timeout waiting for GRETH MII read 2 %d\n",
					/* 7 */  "GRETH: Timeout waiting for GRETH MII read 3 %d\n",
					/* 8 */  "GRETH: Timeout waiting for GRETH MII read 4 %d\n",
					/* 9 */  "GRETH: Timeout waiting for GRETH MII read 5 %d\n",
					/* 10 */ "GRETH: Timeout waiting for GRETH MII read 6 %d\n",
					/* 11 */ "GRETH: Timeout waiting for GRETH MII read 7 %d\n",
					/* 12 */ "GRETH: Timeout waiting for GRETH MII read 8 %d\n",
					/* 13 */ "GRETH: Timeout waiting for GRETH MII read 9 %d\n",
					/* 14 */ "GRETH: Timeout waiting for GRETH MII read 10 %d\n"
				};

        _FUNCDBG("grlib_greth_probe started.\n");

	if (dev == NULL) {
	
		dev = init_etherdev(0, 0);
		if (dev == NULL)
			return -ENOMEM;
	}

	/* Initialize the device structure. 
	 */
	if (dev->priv == NULL) {
		cep = (struct greth_private *) kmalloc(sizeof(*cep), GFP_KERNEL);
		dev->priv = cep;
		if (dev->priv == NULL)
			return -ENOMEM;
	}

 	memset(cep, 0, sizeof(*cep));

        amba_apb_device adev[1];
	i = amba_get_free_apbslv_devices (VENDOR_GAISLER, GAISLER_ETHMAC, adev, 1);
        
	if (i == 0) {
                err = -ENODEV;
                goto err_out_free_dev;
	}
	
	printk("Probing GRETH Ethernet Core at 0x%x\n", adev[0].start);
	
	cep = (struct greth_private *)dev->priv;
	
        /* Get pointer ethernet controller configuration registers.
	 */
	cep->regs = (greth_regs *)(adev[0].start);
	regs = (greth_regs *)(adev[0].start);

	cep->irq = adev[0].irq;

	/* Reset the controller. */
        GRETH_REGSAVE(regs->control, GRETH_RESET);
	
	/* wait max 1s for GRETH core to reset itself */
	wait_loop( (GRETH_REGLOAD(regs->control) & GRETH_RESET), \
	           1, \
	           err_timeout, \
	           timeout_error=1);
	
        /* Get the phy address which assumed to have been set
           correctly with the reset value in hardware */
        cep->phyaddr = (GRETH_REGLOAD(regs->mdio) >> 11) & 0x1F;


        /* Check if mac is gigabit capable */
        cep->gbit_mac = (GRETH_REGLOAD(regs->control) >> 27) & 1;

        /* get phy control register default values */
        wait_read_mii(0x8000, cep->phyaddr, 0, regs, 4, err_timeout, timeout_error=5, timeout_error_desc);

        /* reset PHY and wait for completion */
        write_mii_(cep->phyaddr, 0, 0x8000 | tmp, regs, err_timeout, timeout_error=2, timeout_error_desc);

        wait_read_mii(0x8000, cep->phyaddr, 0, regs, 4, err_timeout, timeout_error=6, timeout_error_desc);
        
        /* Check if PHY is autoneg capable and then determine operating mode, 
           otherwise force it to 10 Mbit halfduplex */
        cep->gb = 0;
        cep->fd = 0;
        cep->sp = 0;
        cep->auto_neg = 0;
        if (!((tmp >> 12) & 1)) {
                write_mii_(cep->phyaddr, 0, 0, regs, err_timeout, timeout_error=3, timeout_error_desc);
        } else {
                /* wait for auto negotiation to complete and then check operating mode */
                cep->auto_neg = 1;
                i = 0;
                while (!( ((tmp=read_mii(cep->phyaddr, 1, regs))>=0) && ((tmp >> 5) & 1) ) ) {
                        for (j = 0; j < 10000; j++) {}
                        i++;
                        if (i > 3000) {
                                printk("Auto negotiation timed out. Selecting default config\n");
                                read_mii_(tmp, cep->phyaddr, 0, regs, err_timeout, timeout_error=8, timeout_error_desc);
                                cep->gb = ((tmp >> 6) & 1) && !((tmp >> 13) & 1);
                                cep->sp = !((tmp >> 6) & 1) && ((tmp >> 13) & 1);
                                cep->fd = (tmp >> 8) & 1;
                                goto auto_neg_done;
                        }
                }
								if ( tmp < 0 ){
									timeout_error = 7;
									timeout_error_desc = tmp;
									goto err_timeout;
								}
                if ((tmp >> 8) & 1) {
                        read_mii_(tmp1, cep->phyaddr, 9, regs, err_timeout, timeout_error=9, timeout_error_desc);
                        read_mii_(tmp2, cep->phyaddr, 10, regs, err_timeout, timeout_error=10, timeout_error_desc);
                        if ( (tmp1 & GRETH_MII_EXTADV_1000FD) &&
                             (tmp2 & GRETH_MII_EXTPRT_1000FD)) {
                                cep->gb = 1;
                                cep->fd = 1;
                        }
                        if ( (tmp1 & GRETH_MII_EXTADV_1000HD) &&
                             (tmp2 & GRETH_MII_EXTPRT_1000HD)) {
                                cep->gb = 1;
                                cep->fd = 0;
                        }
                }
                if ((cep->gb == 0) || ((cep->gb == 1) && (cep->gbit_mac == 0))) {
                        read_mii_(tmp1, cep->phyaddr, 4, regs, err_timeout, timeout_error=11, timeout_error_desc);
                        read_mii_(tmp2, cep->phyaddr, 5, regs, err_timeout, timeout_error=12, timeout_error_desc);
                        if ( (tmp1 & GRETH_MII_100TXFD) &&
                             (tmp2 & GRETH_MII_100TXFD)) {
                                cep->sp = 1;
                                cep->fd = 1;
                        }
                        if ( (tmp1 & GRETH_MII_100TXHD) &&
                             (tmp2 & GRETH_MII_100TXHD)) {
                                cep->sp = 1;
                                cep->fd = 0;
                        }
                        if ( (tmp1 & GRETH_MII_10FD) &&
                             (tmp2 & GRETH_MII_10FD)) {
                                cep->fd = 1;
                        }
                        if ((cep->gb == 1) && (cep->gbit_mac == 0)) {
                                cep->gb = 0;
                                cep->fd = 0;
                                write_mii_(cep->phyaddr, 0, cep->sp << 13, regs, err_timeout, timeout_error=4, timeout_error_desc);
                        }
                }

        }
auto_neg_done:
        printk("%s GRETH Ethermac at [0x%x] irq %d. Running %d Mbps %s duplex\n", cep->gbit_mac ? "10/100/1000":"10/100", \
                                                                                     (unsigned int)(regs), (unsigned int)(cep->irq), \
	                                                                              cep->gb ? 1000:(cep->sp ? 100:10), cep->fd ? "full":"half");
        
        /*Read out PHY info if extended registers are available */
        if (tmp & 1) {
                read_mii_(tmp1, cep->phyaddr, 2, regs, err_timeout, timeout_error=13, timeout_error_desc);
                read_mii_(tmp2, cep->phyaddr, 3, regs, err_timeout, timeout_error=14, timeout_error_desc);
                tmp1 = (tmp1 << 6) | ((tmp2 >> 10) & 0x3F);
                tmp = tmp2 & 0xF;
                
                tmp2 = (tmp2 >> 4) & 0x3F;
                printk("PHY: Vendor %x   Device %x    Revision %d\n", tmp1, tmp2, tmp);
        } else {
                printk("PHY info not available\n");
        }
        
        /* set speed and duplex bits in control register */
        GRETH_REGORIN(regs->control, (cep->gb << 8) | (cep->sp << 7) | (cep->fd << 4)); 


	/* Initialize TXBD pointer
	 */
	cep->tx_bd_base =  (greth_bd *) __get_free_page(GFP_KERNEL);
	GRETH_REGSAVE(regs->tx_desc_p,(int) __pa(cep->tx_bd_base)); 
	tx_bd = (volatile greth_bd *)  cep->tx_bd_base;

	/* Initialize RXBD pointer
	 */	
	cep->rx_bd_base =  (greth_bd *) __get_free_page(GFP_KERNEL);
	GRETH_REGSAVE(regs->rx_desc_p,(int) __pa(cep->rx_bd_base)); 
	rx_bd = (volatile greth_bd *)  cep->rx_bd_base;


	/* Initialize pointers.
	 */
	cep->rx_cur = 0;
	cep->tx_next = 0;
	cep->tx_last = 0;
	cep->tx_free = GRETH_TXBD_NUM;


        for(i = 0, k = 0; i < GRETH_TX_BUF_PAGE_NUM; i++) {
            mem_addr = __get_free_page(GFP_KERNEL);
            for(j = 0; j < GRETH_TX_BUF_PPGAE; j++, k++) {
                tx_bd[k].stat = 0;
                tx_bd[k].addr = __pa(mem_addr); 
                mem_addr += GRETH_TX_BUF_SIZE;
                cep->tx_skbuff[k] = NULL;
            }
        }
        
        if(!(cep->gbit_mac)) {
                for(i = 0, k = 0; i < GRETH_RX_BUF_PAGE_NUM; i++) {
                        mem_addr = __get_free_page(GFP_KERNEL);
                        for(j = 0; j < GRETH_RX_BUF_PPGAE; j++, k++) {
                                rx_bd[k].stat = 0;
                                rx_bd[k].addr = __pa(mem_addr);
                                mem_addr += GRETH_RX_BUF_SIZE;
                                cep->rx_skbuff[k] = NULL;
                        }
                }
        } else {
                for(i = 0; i < GRETH_RXBD_NUM; i++) {
                        rx_bd[i].stat = 0;
                        cep->rx_skbuff[i] = NULL;
                }
        }
        

        /* Set default ethernet station address.
	 */
	dev->dev_addr[0] = MACADDR0;
	dev->dev_addr[1] = MACADDR1;
	dev->dev_addr[2] = MACADDR2;
	dev->dev_addr[3] = MACADDR3;
	dev->dev_addr[4] = MACADDR4;
	dev->dev_addr[5] = MACADDR5;

	GRETH_REGSAVE(regs->esa_msb, MACADDR0 << 8 | MACADDR1); 
	GRETH_REGSAVE(regs->esa_lsb, MACADDR2 << 24 | MACADDR3 << 16 | MACADDR4 << 8 | MACADDR5); 
	
	/* Clear all pending interrupts 
	 */
	GRETH_REGSAVE(regs->status, 0xFF);

	ether_setup(dev);

        dev->base_addr = (unsigned long)adev[0].start;

	/* The GRETH Ethernet MAC specific entries in the device structure. 
	 */
	dev->open = greth_open;
        dev->hard_start_xmit = greth_start_xmit;
        dev->stop = greth_close;
	dev->get_stats = greth_get_stats;
/*	dev->set_multicast_list = greth_set_multicast_list;*/
	dev->flags &= ~IFF_MULTICAST;
	dev->set_mac_address = greth_set_mac_add;

        _FUNCDBG("greth_probe exit\n");
	return 0;

err_timeout:
	err = -EIO;
	printk(timeout_error_str[timeout_error],timeout_error_desc);

err_out_free_dev:
        kfree(dev->priv);
/*        free_netdev(dev); */
        return err;
}

