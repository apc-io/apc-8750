/*++
	linux/include/asm-arm/arch-wmt/entry-macro.S

	Low-level IRQ helper macros for WMT SoC platforms

	Copyright (c) 2008  WonderMedia Technologies, Inc.

	This program is free software: you can redistribute it and/or modify it under the
	terms of the GNU General Public License as published by the Free Software Foundation,
	either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
	PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	You should have received a copy of the GNU General Public License along with
	this program.  If not, see <http://www.gnu.org/licenses/>.

	WonderMedia Technologies, Inc.
	10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

#ifdef IOC_BASE
		.macro	disable_fiq
		.endm

		.macro	get_irqnr_and_base, irqnr, irqstat, base, tmp
		ldr	\base, =IO_ADDRESS(VERSATILE_VIC_BASE)
		ldr	\irqstat, [\base, #VIC_IRQ_STATUS]	@ get masked status
		mov	\irqnr, #0
		teq	\irqstat, #0
		beq	1003f

1001:		tst	\irqstat, #15
		bne	1002f
		add	\irqnr, \irqnr, #4
		movs	\irqstat, \irqstat, lsr #4
		bne	1001b
1002:		tst	\irqstat, #1
		bne	1003f
		add	\irqnr, \irqnr, #1
		movs	\irqstat, \irqstat, lsr #1
		bne	1002b
1003:		/* EQ will be set if no irqs pending */

@		clz	\irqnr, \irqstat
@1003:		/* EQ will be set if we reach MAXIRQNUM */
		.endm
#elif defined(CONFIG_ARCH_WMT)
		.macro	disable_fiq
		@ TBD if need
		.endm

		.macro  get_irqnr_preamble, base, tmp
		.endm

		.macro  arch_ret_to_user, tmp1, tmp2
		.endm

		.macro	get_irqnr_and_base, irqnr, irqstat, base, tmp
		mov	r4,     #0xFE000000
		add	r4, r4, #0x00140000             @ INTC_BASE
		ldr	\irqnr, [r4, #0x00]             @ get zac_irq, ICHS6 = 0xFE140000

	    cmp	\irqnr, #56                   @ Is irq num 56~58?
        cmpne	\irqnr, #57
        cmpne	\irqnr, #58
		moveq	r4,     #0xFE000000
		addeq	r4, r4, #0x00150000	        @ INTC1_BASE
		ldreq	\irqnr, [r4, #0x00]             @ get zac_irq, ICHS6 = 0xFE150000
		mov     \tmp, \irqnr                    @ tmp = logic shift index
		addeq	\irqnr, \irqnr,#64              @ irq number + 64

		add     \irqstat, r4, #0x80             @ save ICIS6A_ADDR to irqstat
		cmp     \tmp, #32                       @ (irqnr >= 32)?
		addhs   \irqstat, \irqstat, #4          @ (irqnr >= 32)? (irqstat+=4):
		                                    	@                (irqstat=irqstat)
		subhs   \tmp, \tmp, #32
		ldr     \irqstat, [\irqstat]            @ get pending zac_irq bitmap
		mov     r4, #1
		mov     r4, r4, lsl \tmp                @ convert tmp to bitmap
		tst     r4, \irqstat                    @ Z set if not valid zac_irq
		.endm

		.macro	irq_prio_table
		@ TBD if need
		.endm
#else
#error Unknown architecture
#endif
