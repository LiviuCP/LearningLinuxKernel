#include <linux/init.h>
#include <linux/kd.h>
#include <linux/tty.h>

#include <linux/console_struct.h>

#include "blinking_led_impl.h"

// bit nr. 2 in a LEDs turned-on/off byte corresponds to the CAPS-LOCK LED
#define CAPS_LOCK_LED_ON 0x04

#define LEDS_OFF 0x00

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(
    "This module implements blinking the CAPS LOCK LED to reproduce the \"S.O.S\" signal in Morse code!\n"
    "Please ensure CAPS-LOCK is off when launching the module otherwise it might not work properly.\n");
MODULE_AUTHOR("Liviu Popa");

extern int fg_console;                    // active TTY number
struct tty_driver* console_driver = NULL; // stores reference to driver of the active console

struct timer_list blinking_led_timer; // timer used for setting up the on/off time intervals for the CAPS-LOCK LED

static void handle_timeout(struct timer_list* timer)
{
    // update the LEDs based on current status
    const unsigned char new_leds_value = should_turn_caps_lock_led_off() ? LEDS_OFF : CAPS_LOCK_LED_ON;
    ((console_driver->ops)->ioctl)(vc_cons[fg_console].d->port.tty, KDSETLED, new_leds_value);

    // schedule the next call (compute timeout period of current status)
    const uint64_t timeout_period = compute_timeout_period();
    timer->expires = jiffies_64 + timeout_period;

    // compute next status, will be used to update LEDs at next timeout
    compute_next_status();

    // restart timer
    add_timer(timer);
}

static int blinking_led_init(void)
{
    pr_info("%s: fg_console is %x\n", THIS_MODULE->name, fg_console);

    console_driver = vc_cons[fg_console].d->port.tty->driver;

    blinking_led_timer.expires = jiffies_64 + 1; // force an immediate timeout
    timer_setup(&blinking_led_timer, handle_timeout, 0);

    add_timer(&blinking_led_timer);

    return 0;
}

static void blinking_led_exit(void)
{
    del_timer(&blinking_led_timer);

    // TODO: implement restoring the leds state to the actual state of the functionality
    // (i.e. if CAPS-LOCK is on when module exits the led should be turned on)
    ((console_driver->ops)->ioctl)(vc_cons[fg_console].d->port.tty, KDSETLED, LEDS_OFF);
}

module_init(blinking_led_init);
module_exit(blinking_led_exit);
