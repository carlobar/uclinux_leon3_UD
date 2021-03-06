/* $Id: avm_pci.c,v 1.1.1.1 1999/11/22 03:47:20 christ Exp $

 * avm_pci.c    low level stuff for AVM Fritz!PCI and ISA PnP isdn cards
 *              Thanks to AVM, Berlin for informations
 *
 * Author       Karsten Keil (keil@isdn4linux.de)
 *
 *
 * $Log: avm_pci.c,v $
 * Revision 1.1.1.1  1999/11/22 03:47:20  christ
 * Importing new-wave v1.0.4
 *
 * Revision 1.1.2.8  1998/11/05 21:11:12  keil
 * AVM PnP support
 *
 * Revision 1.1.2.7  1998/11/03 00:05:48  keil
 * certification related changes
 * fixed logging for smaller stack use
 *
 * Revision 1.1.2.6  1998/10/16 12:46:03  keil
 * fix pci detection for more as one card
 *
 * Revision 1.1.2.5  1998/10/13 18:38:50  keil
 * Fix PCI detection
 *
 * Revision 1.1.2.4  1998/10/04 23:03:41  keil
 * PCI has 255 device entries
 *
 * Revision 1.1.2.3  1998/09/27 23:52:57  keil
 * Fix error handling
 *
 * Revision 1.1.2.2  1998/09/27 13:03:16  keil
 * Fix segfaults on connect
 *
 * Revision 1.1.2.1  1998/08/25 14:01:24  calle
 * Ported driver for AVM Fritz!Card PCI from the 2.1 tree.
 * I could not test it.
 *
 * Revision 1.1  1998/08/20 13:47:30  keil
 * first version
 *
 *
 *
 */
#define __NO_VERSION__
#include <linux/config.h>
#include "hisax.h"
#include "isac.h"
#include "isdnl1.h"
#include <linux/pci.h>
#include <linux/bios32.h>
#include <linux/interrupt.h>

extern const char *CardType[];
static const char *avm_pci_rev = "$Revision: 1.1.1.1 $";

#define  AVM_FRITZ_PCI		1
#define  AVM_FRITZ_PNP		2

#define  PCI_VENDOR_AVM		0x1244
#define  PCI_FRITZPCI_ID	0xa00

#define  HDLC_FIFO		0x0
#define  HDLC_STATUS		0x4

#define	 AVM_HDLC_1		0x00
#define	 AVM_HDLC_2		0x01
#define	 AVM_ISAC_FIFO		0x02
#define	 AVM_ISAC_REG_LOW	0x04
#define	 AVM_ISAC_REG_HIGH	0x06

#define  AVM_STATUS0_IRQ_ISAC	0x01
#define  AVM_STATUS0_IRQ_HDLC	0x02
#define  AVM_STATUS0_IRQ_TIMER	0x04
#define  AVM_STATUS0_IRQ_MASK	0x07

#define  AVM_STATUS0_RESET	0x01
#define  AVM_STATUS0_DIS_TIMER	0x02
#define  AVM_STATUS0_RES_TIMER	0x04
#define  AVM_STATUS0_ENA_IRQ	0x08
#define  AVM_STATUS0_TESTBIT	0x10

#define  AVM_STATUS1_INT_SEL	0x0f
#define  AVM_STATUS1_ENA_IOM	0x80

#define  HDLC_MODE_ITF_FLG	0x01
#define  HDLC_MODE_TRANS	0x02
#define  HDLC_MODE_CCR_7	0x04
#define  HDLC_MODE_CCR_16	0x08
#define  HDLC_MODE_TESTLOOP	0x80

#define  HDLC_INT_XPR		0x80
#define  HDLC_INT_XDU		0x40
#define  HDLC_INT_RPR		0x20
#define  HDLC_INT_MASK		0xE0

#define  HDLC_STAT_RME		0x01
#define  HDLC_STAT_RDO		0x10
#define  HDLC_STAT_CRCVFRRAB	0x0E
#define  HDLC_STAT_CRCVFR	0x06
#define  HDLC_STAT_RML_MASK	0x3f00

#define  HDLC_CMD_XRS		0x80
#define  HDLC_CMD_XME		0x01
#define  HDLC_CMD_RRS		0x20
#define  HDLC_CMD_XML_MASK	0x3f00


/* Interface functions */

static u_char
ReadISAC(struct IsdnCardState *cs, u_char offset)
{
	register u_char idx = (offset > 0x2f) ? AVM_ISAC_REG_HIGH : AVM_ISAC_REG_LOW;
	register u_char val;
	register long flags;

	save_flags(flags);
	cli();
	outb(idx, cs->hw.avm.cfg_reg + 4);
	val = inb(cs->hw.avm.isac + (offset & 0xf));
	restore_flags(flags);
	return (val);
}

static void
WriteISAC(struct IsdnCardState *cs, u_char offset, u_char value)
{
	register u_char idx = (offset > 0x2f) ? AVM_ISAC_REG_HIGH : AVM_ISAC_REG_LOW;
	register long flags;

	save_flags(flags);
	cli();
	outb(idx, cs->hw.avm.cfg_reg + 4);
	outb(value, cs->hw.avm.isac + (offset & 0xf));
	restore_flags(flags);
}

static void
ReadISACfifo(struct IsdnCardState *cs, u_char * data, int size)
{
	outb(AVM_ISAC_FIFO, cs->hw.avm.cfg_reg + 4);
	insb(cs->hw.avm.isac, data, size);
}

static void
WriteISACfifo(struct IsdnCardState *cs, u_char * data, int size)
{
	outb(AVM_ISAC_FIFO, cs->hw.avm.cfg_reg + 4);
	outsb(cs->hw.avm.isac, data, size);
}

static inline u_int
ReadHDLCPCI(struct IsdnCardState *cs, int chan, u_char offset)
{
	register u_int idx = chan ? AVM_HDLC_2 : AVM_HDLC_1;
	register u_int val;
	register long flags;

	save_flags(flags);
	cli();
	outl(idx, cs->hw.avm.cfg_reg + 4);
	val = inl(cs->hw.avm.isac + offset);
	restore_flags(flags);
	return (val);
}

static inline void
WriteHDLCPCI(struct IsdnCardState *cs, int chan, u_char offset, u_int value)
{
	register u_int idx = chan ? AVM_HDLC_2 : AVM_HDLC_1;
	register long flags;

	save_flags(flags);
	cli();
	outl(idx, cs->hw.avm.cfg_reg + 4);
	outl(value, cs->hw.avm.isac + offset);
	restore_flags(flags);
}

static inline u_char
ReadHDLCPnP(struct IsdnCardState *cs, int chan, u_char offset)
{
	register u_char idx = chan ? AVM_HDLC_2 : AVM_HDLC_1;
	register u_char val;
	register long flags;

	save_flags(flags);
	cli();
	outb(idx, cs->hw.avm.cfg_reg + 4);
	val = inb(cs->hw.avm.isac + offset);
	restore_flags(flags);
	return (val);
}

static inline void
WriteHDLCPnP(struct IsdnCardState *cs, int chan, u_char offset, u_char value)
{
	register u_char idx = chan ? AVM_HDLC_2 : AVM_HDLC_1;
	register long flags;

	save_flags(flags);
	cli();
	outb(idx, cs->hw.avm.cfg_reg + 4);
	outb(value, cs->hw.avm.isac + offset);
	restore_flags(flags);
}

static u_char
ReadHDLC_s(struct IsdnCardState *cs, int chan, u_char offset)
{
	return(0xff & ReadHDLCPCI(cs, chan, offset));
}

static void
WriteHDLC_s(struct IsdnCardState *cs, int chan, u_char offset, u_char value)
{
	WriteHDLCPCI(cs, chan, offset, value);
}

static inline
struct BCState *Sel_BCS(struct IsdnCardState *cs, int channel)
{
	if (cs->bcs[0].mode && (cs->bcs[0].channel == channel))
		return(&cs->bcs[0]);
	else if (cs->bcs[1].mode && (cs->bcs[1].channel == channel))
		return(&cs->bcs[1]);
	else
		return(NULL);
}

void inline
hdlc_sched_event(struct BCState *bcs, int event)
{
	bcs->event |= 1 << event;
	queue_task(&bcs->tqueue, &tq_immediate);
	mark_bh(IMMEDIATE_BH);
}

void
write_ctrl(struct BCState *bcs, int which) {

	if (bcs->cs->debug & L1_DEB_HSCX)
		debugl1(bcs->cs, "hdlc %c wr%x ctrl %x",
			'A' + bcs->channel, which, bcs->hw.hdlc.ctrl.ctrl);
	if (bcs->cs->subtyp == AVM_FRITZ_PCI) {
		WriteHDLCPCI(bcs->cs, bcs->channel, HDLC_STATUS, bcs->hw.hdlc.ctrl.ctrl);
	} else {
		if (which & 4)
			WriteHDLCPnP(bcs->cs, bcs->channel, HDLC_STATUS + 2,
				bcs->hw.hdlc.ctrl.sr.mode);
		if (which & 2)
			WriteHDLCPnP(bcs->cs, bcs->channel, HDLC_STATUS + 1,
				bcs->hw.hdlc.ctrl.sr.xml);
		if (which & 1)
			WriteHDLCPnP(bcs->cs, bcs->channel, HDLC_STATUS,
				bcs->hw.hdlc.ctrl.sr.cmd);
	}
}

void
modehdlc(struct BCState *bcs, int mode, int bc)
{
	struct IsdnCardState *cs = bcs->cs;
	int hdlc = bcs->channel;

	if (cs->debug & L1_DEB_HSCX)
		debugl1(cs, "hdlc %c mode %d ichan %d",
			'A' + hdlc, mode, bc);
	bcs->mode = mode;
	bcs->channel = bc;
	bcs->hw.hdlc.ctrl.ctrl = 0;
	switch (mode) {
		case (L1_MODE_NULL):
			bcs->hw.hdlc.ctrl.sr.cmd  = HDLC_CMD_XRS | HDLC_CMD_RRS;
			bcs->hw.hdlc.ctrl.sr.mode = HDLC_MODE_TRANS;
			write_ctrl(bcs, 5);
			break;
		case (L1_MODE_TRANS):
			bcs->hw.hdlc.ctrl.sr.cmd  = HDLC_CMD_XRS | HDLC_CMD_RRS;
			bcs->hw.hdlc.ctrl.sr.mode = HDLC_MODE_TRANS;
			write_ctrl(bcs, 5);
			bcs->hw.hdlc.ctrl.sr.cmd = HDLC_CMD_XRS;
			write_ctrl(bcs, 1);
			bcs->hw.hdlc.ctrl.sr.cmd = 0;
			hdlc_sched_event(bcs, B_XMTBUFREADY);
			break;
		case (L1_MODE_HDLC):
			bcs->hw.hdlc.ctrl.sr.cmd  = HDLC_CMD_XRS | HDLC_CMD_RRS;
			bcs->hw.hdlc.ctrl.sr.mode = HDLC_MODE_ITF_FLG;
			write_ctrl(bcs, 5);
			bcs->hw.hdlc.ctrl.sr.cmd = HDLC_CMD_XRS;
			write_ctrl(bcs, 1);
			bcs->hw.hdlc.ctrl.sr.cmd = 0;
			hdlc_sched_event(bcs, B_XMTBUFREADY);
			break;
	}
}

static inline void
hdlc_empty_fifo(struct BCState *bcs, int count)
{
	register u_int *ptr;
	u_char *p;
	u_char idx = bcs->channel ? AVM_HDLC_2 : AVM_HDLC_1;
	int cnt=0;
	struct IsdnCardState *cs = bcs->cs;

	if ((cs->debug & L1_DEB_HSCX) && !(cs->debug & L1_DEB_HSCX_FIFO))
		debugl1(cs, "hdlc_empty_fifo %d", count);
	if (bcs->hw.hdlc.rcvidx + count > HSCX_BUFMAX) {
		if (cs->debug & L1_DEB_WARN)
			debugl1(cs, "hdlc_empty_fifo: incoming packet too large");
		return;
	}
	ptr = (u_int *) p = bcs->hw.hdlc.rcvbuf + bcs->hw.hdlc.rcvidx;
	bcs->hw.hdlc.rcvidx += count;
	if (cs->subtyp == AVM_FRITZ_PCI) {
		outl(idx, cs->hw.avm.cfg_reg + 4);
		while (cnt < count) {
			*ptr++ = inl(cs->hw.avm.isac);
			cnt += 4;
		}
	} else {
		outb(idx, cs->hw.avm.cfg_reg + 4);
		while (cnt < count) {
			*p++ = inb(cs->hw.avm.isac);
			cnt++;
		}
	}
	if (cs->debug & L1_DEB_HSCX_FIFO) {
		char *t = bcs->blog;

		if (cs->subtyp == AVM_FRITZ_PNP)
			p = (u_char *) ptr;
		t += sprintf(t, "hdlc_empty_fifo %c cnt %d",
			     bcs->channel ? 'B' : 'A', count);
		QuickHex(t, p, count);
		debugl1(cs, bcs->blog);
	}
}

static inline void
hdlc_fill_fifo(struct BCState *bcs)
{
	struct IsdnCardState *cs = bcs->cs;
	int count, cnt =0;
	int fifo_size = 32;
	u_char *p;
	u_int *ptr;

	if ((cs->debug & L1_DEB_HSCX) && !(cs->debug & L1_DEB_HSCX_FIFO))
		debugl1(cs, "hdlc_fill_fifo");
	if (!bcs->tx_skb)
		return;
	if (bcs->tx_skb->len <= 0)
		return;

	bcs->hw.hdlc.ctrl.sr.cmd &= ~HDLC_CMD_XME;
	if (bcs->tx_skb->len > fifo_size) {
		count = fifo_size;
	} else {
		count = bcs->tx_skb->len;
		if (bcs->mode != L1_MODE_TRANS)
			bcs->hw.hdlc.ctrl.sr.cmd |= HDLC_CMD_XME;
	}
	if ((cs->debug & L1_DEB_HSCX) && !(cs->debug & L1_DEB_HSCX_FIFO))
		debugl1(cs, "hdlc_fill_fifo %d/%ld", count, bcs->tx_skb->len);
	ptr = (u_int *) p = bcs->tx_skb->data;
	skb_pull(bcs->tx_skb, count);
	bcs->tx_cnt -= count;
	bcs->hw.hdlc.count += count;
	bcs->hw.hdlc.ctrl.sr.xml = ((count == fifo_size) ? 0 : count);
	write_ctrl(bcs, 3);  /* sets the correct index too */
	if (cs->subtyp == AVM_FRITZ_PCI) {
		while (cnt<count) {
			outl(*ptr++, cs->hw.avm.isac);
			cnt += 4;
		}
	} else {
		while (cnt<count) {
			outb(*p++, cs->hw.avm.isac);
			cnt++;
		}
	}
	if (cs->debug & L1_DEB_HSCX_FIFO) {
		char *t = bcs->blog;

		if (cs->subtyp == AVM_FRITZ_PNP)
			p = (u_char *) ptr;
		t += sprintf(t, "hdlc_fill_fifo %c cnt %d",
			     bcs->channel ? 'B' : 'A', count);
		QuickHex(t, p, count);
		debugl1(cs, bcs->blog);
	}
}

static void
fill_hdlc(struct BCState *bcs)
{
	long flags;
	save_flags(flags);
	cli();
	hdlc_fill_fifo(bcs);
	restore_flags(flags);
}

static inline void
HDLC_irq(struct BCState *bcs, u_int stat) {
	int len;
	struct sk_buff *skb;

	if (bcs->cs->debug & L1_DEB_HSCX)
		debugl1(bcs->cs, "ch%d stat %#x", bcs->channel, stat);
	if (stat & HDLC_INT_RPR) {
		if (stat & HDLC_STAT_RDO) {
			if (bcs->cs->debug & L1_DEB_HSCX)
				debugl1(bcs->cs, "RDO");
			else
				debugl1(bcs->cs, "ch%d stat %#x", bcs->channel, stat);
			bcs->hw.hdlc.ctrl.sr.xml = 0;
			bcs->hw.hdlc.ctrl.sr.cmd |= HDLC_CMD_RRS;
			write_ctrl(bcs, 1);
			bcs->hw.hdlc.ctrl.sr.cmd &= ~HDLC_CMD_RRS;
			write_ctrl(bcs, 1);
			bcs->hw.hdlc.rcvidx = 0;
		} else {
			if (!(len = (stat & HDLC_STAT_RML_MASK)>>8))
				len = 32;
			hdlc_empty_fifo(bcs, len);
			if ((stat & HDLC_STAT_RME) || (bcs->mode == L1_MODE_TRANS)) {
				if (((stat & HDLC_STAT_CRCVFRRAB)==HDLC_STAT_CRCVFR) ||
					(bcs->mode == L1_MODE_TRANS)) {
					if (!(skb = dev_alloc_skb(bcs->hw.hdlc.rcvidx)))
						printk(KERN_WARNING "HDLC: receive out of memory\n");
					else {
						memcpy(skb_put(skb, bcs->hw.hdlc.rcvidx),
							bcs->hw.hdlc.rcvbuf, bcs->hw.hdlc.rcvidx);
						skb_queue_tail(&bcs->rqueue, skb);
					}
					bcs->hw.hdlc.rcvidx = 0;
					hdlc_sched_event(bcs, B_RCVBUFREADY);
				} else {
					if (bcs->cs->debug & L1_DEB_HSCX)
						debugl1(bcs->cs, "invalid frame");
					else
						debugl1(bcs->cs, "ch%d invalid frame %#x", bcs->channel, stat);
					bcs->hw.hdlc.rcvidx = 0;
				}
			}
		}
	}
	if (stat & HDLC_INT_XDU) {
		/* Here we lost an TX interrupt, so
		 * restart transmitting the whole frame.
		 */
		if (bcs->tx_skb) {
			skb_push(bcs->tx_skb, bcs->hw.hdlc.count);
			bcs->tx_cnt += bcs->hw.hdlc.count;
			bcs->hw.hdlc.count = 0;
//			hdlc_sched_event(bcs, B_XMTBUFREADY);
			if (bcs->cs->debug & L1_DEB_WARN)
				debugl1(bcs->cs, "ch%d XDU", bcs->channel);
		} else if (bcs->cs->debug & L1_DEB_WARN)
			debugl1(bcs->cs, "ch%d XDU without skb", bcs->channel);
		bcs->hw.hdlc.ctrl.sr.xml = 0;
		bcs->hw.hdlc.ctrl.sr.cmd |= HDLC_CMD_XRS;
		write_ctrl(bcs, 1);
		bcs->hw.hdlc.ctrl.sr.cmd &= ~HDLC_CMD_XRS;
		write_ctrl(bcs, 1);
		hdlc_fill_fifo(bcs);
	} else if (stat & HDLC_INT_XPR) {
		if (bcs->tx_skb) {
			if (bcs->tx_skb->len) {
				hdlc_fill_fifo(bcs);
				return;
			} else {
				if (bcs->st->lli.l1writewakeup &&
					(PACKET_NOACK != bcs->tx_skb->pkt_type))
					bcs->st->lli.l1writewakeup(bcs->st, bcs->hw.hdlc.count);
				dev_kfree_skb(bcs->tx_skb, FREE_WRITE);
				bcs->hw.hdlc.count = 0;
				bcs->tx_skb = NULL;
			}
		}
		if ((bcs->tx_skb = skb_dequeue(&bcs->squeue))) {
			bcs->hw.hdlc.count = 0;
			test_and_set_bit(BC_FLG_BUSY, &bcs->Flag);
			hdlc_fill_fifo(bcs);
		} else {
			test_and_clear_bit(BC_FLG_BUSY, &bcs->Flag);
			hdlc_sched_event(bcs, B_XMTBUFREADY);
		}
	}
}

inline void
HDLC_irq_main(struct IsdnCardState *cs)
{
	u_int stat;
	long  flags;
	struct BCState *bcs;

	save_flags(flags);
	cli();
	if (cs->subtyp == AVM_FRITZ_PCI) {
		stat = ReadHDLCPCI(cs, 0, HDLC_STATUS);
	} else {
		stat = ReadHDLCPnP(cs, 0, HDLC_STATUS);
		if (stat & HDLC_INT_RPR)
			stat |= (ReadHDLCPnP(cs, 0, HDLC_STATUS+1))<<8;
	}
	if (stat & HDLC_INT_MASK) {
		if (!(bcs = Sel_BCS(cs, 0))) {
			if (cs->debug)
				debugl1(cs, "hdlc spurious channel 0 IRQ");
		} else
			HDLC_irq(bcs, stat);
	}
	if (cs->subtyp == AVM_FRITZ_PCI) {
		stat = ReadHDLCPCI(cs, 1, HDLC_STATUS);
	} else {
		stat = ReadHDLCPnP(cs, 1, HDLC_STATUS);
		if (stat & HDLC_INT_RPR)
			stat |= (ReadHDLCPnP(cs, 1, HDLC_STATUS+1))<<8;
	}
	if (stat & HDLC_INT_MASK) {
		if (!(bcs = Sel_BCS(cs, 1))) {
			if (cs->debug)
				debugl1(cs, "hdlc spurious channel 1 IRQ");
		} else
			HDLC_irq(bcs, stat);
	}
	restore_flags(flags);
}

void
hdlc_l2l1(struct PStack *st, int pr, void *arg)
{
	struct sk_buff *skb = arg;
	long flags;

	switch (pr) {
		case (PH_DATA | REQUEST):
			save_flags(flags);
			cli();
			if (st->l1.bcs->tx_skb) {
				skb_queue_tail(&st->l1.bcs->squeue, skb);
				restore_flags(flags);
			} else {
				st->l1.bcs->tx_skb = skb;
				test_and_set_bit(BC_FLG_BUSY, &st->l1.bcs->Flag);
				st->l1.bcs->hw.hdlc.count = 0;
				restore_flags(flags);
				st->l1.bcs->cs->BC_Send_Data(st->l1.bcs);
			}
			break;
		case (PH_PULL | INDICATION):
			if (st->l1.bcs->tx_skb) {
				printk(KERN_WARNING "hdlc_l2l1: this shouldn't happen\n");
				break;
			}
			test_and_set_bit(BC_FLG_BUSY, &st->l1.bcs->Flag);
			st->l1.bcs->tx_skb = skb;
			st->l1.bcs->hw.hdlc.count = 0;
			st->l1.bcs->cs->BC_Send_Data(st->l1.bcs);
			break;
		case (PH_PULL | REQUEST):
			if (!st->l1.bcs->tx_skb) {
				test_and_clear_bit(FLG_L1_PULL_REQ, &st->l1.Flags);
				st->l1.l1l2(st, PH_PULL | CONFIRM, NULL);
			} else
				test_and_set_bit(FLG_L1_PULL_REQ, &st->l1.Flags);
			break;
		case (PH_ACTIVATE | REQUEST):
			test_and_set_bit(BC_FLG_ACTIV, &st->l1.bcs->Flag);
			modehdlc(st->l1.bcs, st->l1.mode, st->l1.bc);
			l1_msg_b(st, pr, arg);
			break;
		case (PH_DEACTIVATE | REQUEST):
			l1_msg_b(st, pr, arg);
			break;
		case (PH_DEACTIVATE | CONFIRM):
			test_and_clear_bit(BC_FLG_ACTIV, &st->l1.bcs->Flag);
			test_and_clear_bit(BC_FLG_BUSY, &st->l1.bcs->Flag);
			modehdlc(st->l1.bcs, 0, st->l1.bc);
			st->l1.l1l2(st, PH_DEACTIVATE | CONFIRM, NULL);
			break;
	}
}

void
close_hdlcstate(struct BCState *bcs)
{
	modehdlc(bcs, 0, 0);
	if (test_and_clear_bit(BC_FLG_INIT, &bcs->Flag)) {
		if (bcs->hw.hdlc.rcvbuf) {
			kfree(bcs->hw.hdlc.rcvbuf);
			bcs->hw.hdlc.rcvbuf = NULL;
		}
		if (bcs->blog) {
			kfree(bcs->blog);
			bcs->blog = NULL;
		}
		discard_queue(&bcs->rqueue);
		discard_queue(&bcs->squeue);
		if (bcs->tx_skb) {
			dev_kfree_skb(bcs->tx_skb, FREE_WRITE);
			bcs->tx_skb = NULL;
			test_and_clear_bit(BC_FLG_BUSY, &bcs->Flag);
		}
	}
}

int
open_hdlcstate(struct IsdnCardState *cs, struct BCState *bcs)
{
	if (!test_and_set_bit(BC_FLG_INIT, &bcs->Flag)) {
		if (!(bcs->hw.hdlc.rcvbuf = kmalloc(HSCX_BUFMAX, GFP_ATOMIC))) {
			printk(KERN_WARNING
			       "HiSax: No memory for hdlc.rcvbuf\n");
			return (1);
		}
		if (!(bcs->blog = kmalloc(MAX_BLOG_SPACE, GFP_ATOMIC))) {
			printk(KERN_WARNING
				"HiSax: No memory for bcs->blog\n");
			test_and_clear_bit(BC_FLG_INIT, &bcs->Flag);
			kfree(bcs->hw.hdlc.rcvbuf);
			bcs->hw.hdlc.rcvbuf = NULL;
			return (2);
		}
		skb_queue_head_init(&bcs->rqueue);
		skb_queue_head_init(&bcs->squeue);
	}
	bcs->tx_skb = NULL;
	test_and_clear_bit(BC_FLG_BUSY, &bcs->Flag);
	bcs->event = 0;
	bcs->hw.hdlc.rcvidx = 0;
	bcs->tx_cnt = 0;
	return (0);
}

int
setstack_hdlc(struct PStack *st, struct BCState *bcs)
{
	bcs->channel = st->l1.bc;
	if (open_hdlcstate(st->l1.hardware, bcs))
		return (-1);
	st->l1.bcs = bcs;
	st->l2.l2l1 = hdlc_l2l1;
	setstack_manager(st);
	bcs->st = st;
	setstack_l1_B(st);
	return (0);
}

HISAX_INITFUNC(void
clear_pending_hdlc_ints(struct IsdnCardState *cs))
{
	u_int val;

	if (cs->subtyp == AVM_FRITZ_PCI) {
		val = ReadHDLCPCI(cs, 0, HDLC_STATUS);
		debugl1(cs, "HDLC 1 STA %x", val);
		val = ReadHDLCPCI(cs, 1, HDLC_STATUS);
		debugl1(cs, "HDLC 2 STA %x", val);
	} else {
		val = ReadHDLCPnP(cs, 0, HDLC_STATUS);
		debugl1(cs, "HDLC 1 STA %x", val);
		val = ReadHDLCPnP(cs, 0, HDLC_STATUS + 1);
		debugl1(cs, "HDLC 1 RML %x", val);
		val = ReadHDLCPnP(cs, 0, HDLC_STATUS + 2);
		debugl1(cs, "HDLC 1 MODE %x", val);
		val = ReadHDLCPnP(cs, 0, HDLC_STATUS + 3);
		debugl1(cs, "HDLC 1 VIN %x", val);
		val = ReadHDLCPnP(cs, 1, HDLC_STATUS);
		debugl1(cs, "HDLC 2 STA %x", val);
		val = ReadHDLCPnP(cs, 1, HDLC_STATUS + 1);
		debugl1(cs, "HDLC 2 RML %x", val);
		val = ReadHDLCPnP(cs, 1, HDLC_STATUS + 2);
		debugl1(cs, "HDLC 2 MODE %x", val);
		val = ReadHDLCPnP(cs, 1, HDLC_STATUS + 3);
		debugl1(cs, "HDLC 2 VIN %x", val);
	}
}

HISAX_INITFUNC(void
inithdlc(struct IsdnCardState *cs))
{
	cs->bcs[0].BC_SetStack = setstack_hdlc;
	cs->bcs[1].BC_SetStack = setstack_hdlc;
	cs->bcs[0].BC_Close = close_hdlcstate;
	cs->bcs[1].BC_Close = close_hdlcstate;
	modehdlc(cs->bcs, 0, 0);
	modehdlc(cs->bcs + 1, 0, 0);
}

static void
avm_pcipnp_interrupt(int intno, void *dev_id, struct pt_regs *regs)
{
	struct IsdnCardState *cs = dev_id;
	u_char val, stat = 0;
	u_char sval;

	if (!cs) {
		printk(KERN_WARNING "AVM PCI: Spurious interrupt!\n");
		return;
	}
	sval = inb(cs->hw.avm.cfg_reg + 2);
	if ((sval & AVM_STATUS0_IRQ_MASK) == AVM_STATUS0_IRQ_MASK)
		/* possible a shared  IRQ reqest */
		return;
	if (!(sval & AVM_STATUS0_IRQ_ISAC)) {
		val = ReadISAC(cs, ISAC_ISTA);
		isac_interrupt(cs, val);
		stat |= 2;
	}
	if (!(sval & AVM_STATUS0_IRQ_HDLC)) {
		HDLC_irq_main(cs);
	}
	if (stat & 2) {
		WriteISAC(cs, ISAC_MASK, 0xFF);
		WriteISAC(cs, ISAC_MASK, 0x0);
	}
}

static void
reset_avmpcipnp(struct IsdnCardState *cs)
{
	long flags;

	printk(KERN_INFO "AVM PCI/PnP: reset\n");
	save_flags(flags);
	sti();
	outb(AVM_STATUS0_RESET | AVM_STATUS0_DIS_TIMER, cs->hw.avm.cfg_reg + 2);
	current->state = TASK_INTERRUPTIBLE;
	current->timeout = jiffies + (10 * HZ) / 1000;	/* Timeout 10ms */
	schedule();
	outb(AVM_STATUS0_DIS_TIMER | AVM_STATUS0_RES_TIMER | AVM_STATUS0_ENA_IRQ, cs->hw.avm.cfg_reg + 2);
	outb(AVM_STATUS1_ENA_IOM | cs->irq, cs->hw.avm.cfg_reg + 3);
	current->state = TASK_INTERRUPTIBLE;
	current->timeout = jiffies + (10 * HZ) / 1000;	/* Timeout 10ms */
	schedule();
	printk(KERN_INFO "AVM PCI/PnP: S1 %x\n", inb(cs->hw.avm.cfg_reg + 3));
}

static int
AVM_card_msg(struct IsdnCardState *cs, int mt, void *arg)
{
	u_int irq_flag;

	switch (mt) {
		case CARD_RESET:
			reset_avmpcipnp(cs);
			return(0);
		case CARD_RELEASE:
			outb(0, cs->hw.avm.cfg_reg + 2);
			release_region(cs->hw.avm.cfg_reg, 32);
			return(0);
		case CARD_SETIRQ:
			if (cs->subtyp == AVM_FRITZ_PCI)
				irq_flag = I4L_IRQ_FLAG | SA_SHIRQ;
			else
				irq_flag = I4L_IRQ_FLAG;
			return(request_irq(cs->irq, &avm_pcipnp_interrupt,
					irq_flag, "HiSax", cs));
		case CARD_INIT:
			clear_pending_isac_ints(cs);
			initisac(cs);
			clear_pending_hdlc_ints(cs);
			inithdlc(cs);
			outb(AVM_STATUS0_DIS_TIMER | AVM_STATUS0_RES_TIMER,
				cs->hw.avm.cfg_reg + 2);
			WriteISAC(cs, ISAC_MASK, 0);
			outb(AVM_STATUS0_DIS_TIMER | AVM_STATUS0_RES_TIMER |
				AVM_STATUS0_ENA_IRQ, cs->hw.avm.cfg_reg + 2);
			/* RESET Receiver and Transmitter */
			WriteISAC(cs, ISAC_CMDR, 0x41);
			return(0);
		case CARD_TEST:
			return(0);
	}
	return(0);
}

static 	int pci_index __initdata = 0;

__initfunc(int
setup_avm_pcipnp(struct IsdnCard *card))
{
	u_int val, ver;
	struct IsdnCardState *cs = card->cs;
	char tmp[64];

	strcpy(tmp, avm_pci_rev);
	printk(KERN_INFO "HiSax: AVM PCI/ISAPnP driver Rev. %s\n", HiSax_getrev(tmp));
	if (cs->typ != ISDN_CTYPE_FRITZPCI)
		return (0);
	if (card->para[1]) {
		cs->hw.avm.cfg_reg = card->para[1];
		cs->irq = card->para[0];
		cs->subtyp = AVM_FRITZ_PNP;
	} else {
#if CONFIG_PCI
		for (; pci_index < 255; pci_index++) {
			unsigned char pci_bus, pci_device_fn;
			unsigned int ioaddr;
			unsigned char irq;

			if (pcibios_find_device (PCI_VENDOR_AVM,
				PCI_FRITZPCI_ID, pci_index,
				&pci_bus, &pci_device_fn) != 0) {
				continue;
			}
			pcibios_read_config_byte(pci_bus, pci_device_fn,
				PCI_INTERRUPT_LINE, &irq);
			pcibios_read_config_dword(pci_bus, pci_device_fn,
				PCI_BASE_ADDRESS_1, &ioaddr);
			cs->irq = irq;
			cs->hw.avm.cfg_reg = ioaddr & PCI_BASE_ADDRESS_IO_MASK;
			if (!cs->hw.avm.cfg_reg) {
				printk(KERN_WARNING "FritzPCI: No IO-Adr for PCI card found\n");
				return(0);
			}
			cs->subtyp = AVM_FRITZ_PCI;
			break;
		}
		if (pci_index == 255) {
			printk(KERN_WARNING "FritzPCI: No PCI card found\n");
			return(0);
        	}
	        pci_index++;
#else
		printk(KERN_WARNING "FritzPCI: NO_PCI_BIOS\n");
		return (0);
#endif /* CONFIG_PCI */
	}
	cs->hw.avm.isac = cs->hw.avm.cfg_reg + 0x10;
	if (check_region((cs->hw.avm.cfg_reg), 32)) {
		printk(KERN_WARNING
		       "HiSax: %s config port %x-%x already in use\n",
		       CardType[card->typ],
		       cs->hw.avm.cfg_reg,
		       cs->hw.avm.cfg_reg + 31);
		return (0);
	} else {
		request_region(cs->hw.avm.cfg_reg, 32,
			(cs->subtyp == AVM_FRITZ_PCI) ? "avm PCI" : "avm PnP");
	}
	switch (cs->subtyp) {
	  case AVM_FRITZ_PCI:
		val = inl(cs->hw.avm.cfg_reg);
		printk(KERN_INFO "AVM PCI: stat %#x\n", val);
		printk(KERN_INFO "AVM PCI: Class %X Rev %d\n",
			val & 0xff, (val>>8) & 0xff);
		cs->BC_Read_Reg = &ReadHDLC_s;
		cs->BC_Write_Reg = &WriteHDLC_s;
		break;
	  case AVM_FRITZ_PNP:
		val = inb(cs->hw.avm.cfg_reg);
		ver = inb(cs->hw.avm.cfg_reg + 1);
		printk(KERN_INFO "AVM PnP: Class %X Rev %d\n", val, ver);
		reset_avmpcipnp(cs);
		cs->BC_Read_Reg = &ReadHDLCPnP;
		cs->BC_Write_Reg = &WriteHDLCPnP;
		break;
	  default:
	  	printk(KERN_WARNING "AVM unknown subtype %d\n", cs->subtyp);
	  	return(0);
	}
	printk(KERN_INFO "HiSax: %s config irq:%d base:0x%X\n",
		(cs->subtyp == AVM_FRITZ_PCI) ? "AVM Fritz!PCI" : "AVM Fritz!PnP",
		cs->irq, cs->hw.avm.cfg_reg);

	cs->readisac = &ReadISAC;
	cs->writeisac = &WriteISAC;
	cs->readisacfifo = &ReadISACfifo;
	cs->writeisacfifo = &WriteISACfifo;
	cs->BC_Send_Data = &fill_hdlc;
	cs->cardmsg = &AVM_card_msg;
	ISACVersion(cs, (cs->subtyp == AVM_FRITZ_PCI) ? "AVM PCI:" : "AVM PnP:");
	return (1);
}
