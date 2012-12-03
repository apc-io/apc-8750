/*++
linux/arch/arm/mach-wmt/leds-wmt.c

WMT leds events
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

#include <linux/init.h>

#include <mach/hardware.h>
#include <asm/leds.h>
#include <asm/system.h>

#include "leds.h"


#define LED_STATE_ENABLED       1
#define LED_STATE_CLAIMED       2

/*
 * Specify your LEDs of system here
 */
#define LED_RED            (1 << 0)     /* no RED LED*/
#define LED_GREEN          (1 << 1)     /* no GREEN LED*/
#define LED_AMBER          (1 << 2)     /* no AMBER LED*/
#define LED_BLUE           (1 << 3)     /* GPIO_0*/


#define LED_CPU                 LED_RED
#define LED_HALT                LED_BLUE
#define LED_TIMER               LED_GREEN

#define LED_MASK                LED_BLUE

static unsigned int led_state;
static unsigned int hw_led_state;

void wmt_leds_event(led_event_t evt)
{
	unsigned long flags;

	local_irq_save(flags);
	switch (evt) {
	case led_start:
		led_state = LED_STATE_ENABLED;
		hw_led_state = LED_MASK;
		gpio_enable(GPIO_LED_BLUE, GPIO_OUTPUT_MODE);
	break;

	case led_stop:
		led_state &= ~LED_STATE_ENABLED;
	break;

	case led_claim:
		led_state |= LED_STATE_CLAIMED;
		hw_led_state = LED_MASK;
	break;

	case led_release:
		led_state &= ~LED_STATE_CLAIMED;
		hw_led_state = LED_MASK;
	break;

#ifdef CONFIG_LEDS_TIMER
	case led_timer:
		/* We will be entered with IRQs enabled. */
		if (!(led_state & LED_STATE_CLAIMED))
			hw_led_state ^= LED_TIMER;
	break;
#endif

#ifdef CONFIG_LEDS_CPU
	case led_idle_start:
		if (!(led_state & LED_STATE_CLAIMED))
			hw_led_state |= LED_CPU;
	break;

	case led_idle_end:
		if (!(led_state & LED_STATE_CLAIMED))
			hw_led_state &= ~LED_CPU;
	break;
#endif

#ifdef CONFIG_LEDS_HALT
	case led_halted:
		led_state = 0;
		hw_led_state = LED_MASK & ~LED_HALT;
		if (hw_led_state & LED_BLUE)
			gpio_set_value(GPIO_LED_BLUE, GPIO_OUTPUT_HIGH);
		else
			gpio_set_value(GPIO_LED_BLUE, GPIO_OUTPUT_LOW);
	break;
#endif
	case led_amber_on:
		if (led_state & LED_STATE_CLAIMED)
			hw_led_state &= ~LED_AMBER;
	break;

	case led_amber_off:
		if (led_state & LED_STATE_CLAIMED)
			hw_led_state |= LED_AMBER;
	break;
	case led_blue_on:
		if (led_state & LED_STATE_CLAIMED)
			hw_led_state &= ~LED_BLUE;
	break;

	case led_blue_off:
		if (led_state & LED_STATE_CLAIMED)
			hw_led_state |= LED_BLUE;
	break;
	case led_red_on:
		if (led_state & LED_STATE_CLAIMED)
			hw_led_state &= ~LED_RED;
	break;

	case led_red_off:
		if (led_state & LED_STATE_CLAIMED)
			hw_led_state |= LED_RED;
	break;
	case led_green_on:
		if (led_state & LED_STATE_CLAIMED)
			hw_led_state &= ~LED_GREEN;
	break;

	case led_green_off:
		if (led_state & LED_STATE_CLAIMED)
			hw_led_state |= LED_GREEN;
	break;
	default:
	break;
	}

	if (led_state & LED_STATE_ENABLED) {
		if (hw_led_state & LED_BLUE)
			gpio_set_value(GPIO_LED_BLUE, GPIO_OUTPUT_HIGH);
		else
			gpio_set_value(GPIO_LED_BLUE, GPIO_OUTPUT_LOW);

	}
	local_irq_restore(flags);
}
