/*
 * Copyright (c) 2009 Daniel Mack <daniel@caiaq.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <common.h>
#include <usb.h>
#include <asm/io.h>
#include <asm/arch/mx31-regs.h>
#include <usb/ehci-fsl.h>
#include <errno.h>

#include "ehci.h"
#include "ehci-core.h"

#define USBCTRL_OTGBASE_OFFSET	0x600

#define MX31_OTG_SIC_SHIFT	29
#define MX31_OTG_SIC_MASK	(0x3 << MX31_OTG_SIC_SHIFT)
#define MX31_OTG_PM_BIT		(1 << 24)

#define MX31_H2_SIC_SHIFT	21
#define MX31_H2_SIC_MASK	(0x3 << MX31_H2_SIC_SHIFT)
#define MX31_H2_PM_BIT		(1 << 16)
#define MX31_H2_DT_BIT		(1 << 5)

#define MX31_H1_SIC_SHIFT	13
#define MX31_H1_SIC_MASK	(0x3 << MX31_H1_SIC_SHIFT)
#define MX31_H1_PM_BIT		(1 << 8)
#define MX31_H1_DT_BIT		(1 << 4)

static int mxc_set_usbcontrol(int port, unsigned int flags)
{
	unsigned int v;
#ifdef CONFIG_MX31
		v = readl(MX31_OTG_BASE_ADDR + USBCTRL_OTGBASE_OFFSET);

		switch (port) {
		case 0:	/* OTG port */
			v &= ~(MX31_OTG_SIC_MASK | MX31_OTG_PM_BIT);
			v |= (flags & MXC_EHCI_INTERFACE_MASK)
					<< MX31_OTG_SIC_SHIFT;
			if (!(flags & MXC_EHCI_POWER_PINS_ENABLED))
				v |= MX31_OTG_PM_BIT;

			break;
		case 1: /* H1 port */
			v &= ~(MX31_H1_SIC_MASK | MX31_H1_PM_BIT |
				MX31_H1_DT_BIT);
			v |= (flags & MXC_EHCI_INTERFACE_MASK)
						<< MX31_H1_SIC_SHIFT;
			if (!(flags & MXC_EHCI_POWER_PINS_ENABLED))
				v |= MX31_H1_PM_BIT;

			if (!(flags & MXC_EHCI_TTL_ENABLED))
				v |= MX31_H1_DT_BIT;

			break;
		case 2:	/* H2 port */
			v &= ~(MX31_H2_SIC_MASK | MX31_H2_PM_BIT |
				MX31_H2_DT_BIT);
			v |= (flags & MXC_EHCI_INTERFACE_MASK)
						<< MX31_H2_SIC_SHIFT;
			if (!(flags & MXC_EHCI_POWER_PINS_ENABLED))
				v |= MX31_H2_PM_BIT;

			if (!(flags & MXC_EHCI_TTL_ENABLED))
				v |= MX31_H2_DT_BIT;

			break;
		default:
			return -EINVAL;
		}

		writel(v, MX31_OTG_BASE_ADDR +
				     USBCTRL_OTGBASE_OFFSET);
#endif
		return 0;
}

int ehci_hcd_init(void)
{
	u32 tmp;
	struct usb_ehci *ehci;
	struct clock_control_regs *sc_regs =
		(struct clock_control_regs *)CCM_BASE;

	tmp = __raw_readl(&sc_regs->ccmr);
	__raw_writel(__raw_readl(&sc_regs->ccmr) | (1 << 9), &sc_regs->ccmr) ;

	udelay(80);

	/* Take USB2 */
	ehci = (struct usb_ehci *)(MX31_OTG_BASE_ADDR +
		(0x200 * CONFIG_MXC_USB_PORT));
	hccr = (struct ehci_hccr *)((uint32_t)&ehci->caplength);
	hcor = (struct ehci_hcor *)((uint32_t) hccr +
			HC_LENGTH(ehci_readl(&hccr->cr_capbase)));
	setbits_le32(&ehci->usbmode, CM_HOST);
	setbits_le32(&ehci->control, USB_EN);

	__raw_writel(CONFIG_MXC_USB_PORTSC, &ehci->portsc);

	mxc_set_usbcontrol(CONFIG_MXC_USB_PORT, CONFIG_MXC_USB_FLAGS);

	udelay(10000);

	return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(void)
{
	return 0;
}
