#include <linux/module.h>

#include "blinking_led_impl.h"

#define DOT_TURNED_ON_PERIOD HZ / 6   // LED turned on for a "dot" ('S')
#define DOT_TURNED_OFF_PERIOD HZ / 9  // LED turned off after a "dot"
#define LINE_TURNED_ON_PERIOD HZ / 2  // LED turned on for a "line" ('O')
#define LINE_TURNED_OFF_PERIOD HZ / 3 // LED turned off after a "line"
#define IDLE_PERIOD HZ                // LED turned off (break) between two "S.O.S" sequences

#define SHOULD_TURN_OFF_CAPS_LOCK_MASK 0b00000001

enum SequenceState
{
    DOT1 = 0,
    DOT2,
    DOT3,
    LINE1,
    LINE2,
    LINE3,
    DOT4,
    DOT5,
    DOT6,
    IDLE
};

// the status consists of sequence state and the on/off flag for the CAPS-LOCK LED
// initial status: idle period with CAPS-LOCK led turned off so the user can easily observe the first "S.O.S." sequence
static unsigned char status = (IDLE << 1) | SHOULD_TURN_OFF_CAPS_LOCK_MASK;

static unsigned char compose_status(unsigned char sequence_state, bool should_turn_led_off)
{
    return ((sequence_state << 1) | (should_turn_led_off ? SHOULD_TURN_OFF_CAPS_LOCK_MASK : 0));
}

static unsigned char get_sequence_state(void)
{
    return (status & ~SHOULD_TURN_OFF_CAPS_LOCK_MASK) >> 1;
}

bool should_turn_caps_lock_led_off(void)
{
    return status & SHOULD_TURN_OFF_CAPS_LOCK_MASK;
}

// implementation of the "S.O.S" signal - 3 dots, 3 lines, 3 dots
uint64_t compute_timeout_period(void)
{
    const bool should_turn_caps_lock_off = should_turn_caps_lock_led_off();
    const unsigned char sequence_state = get_sequence_state();

    pr_debug("%s: current sequence state: %d, CAPS-LOCK led on: %d", THIS_MODULE->name, sequence_state,
             !should_turn_caps_lock_off);

    uint64_t timeout_period = IDLE_PERIOD;

    switch (sequence_state)
    {
    case DOT1:
    case DOT2: {
        timeout_period = should_turn_caps_lock_off ? DOT_TURNED_OFF_PERIOD : DOT_TURNED_ON_PERIOD;
        break;
    }
    case DOT3: {
        // set a longer off period for third dot to better observe the lines
        timeout_period =
            should_turn_caps_lock_off ? DOT_TURNED_OFF_PERIOD + LINE_TURNED_OFF_PERIOD : DOT_TURNED_ON_PERIOD;
        break;
    }
    case LINE1:
    case LINE2: {
        timeout_period = should_turn_caps_lock_off ? LINE_TURNED_OFF_PERIOD : LINE_TURNED_ON_PERIOD;
        break;
    }
    case LINE3: {
        // set a longer off period for third line to better observe the next dots
        timeout_period =
            should_turn_caps_lock_off ? LINE_TURNED_OFF_PERIOD + DOT_TURNED_OFF_PERIOD : LINE_TURNED_ON_PERIOD;
        break;
    }
    case DOT4:
    case DOT5:
    case DOT6: {
        timeout_period = should_turn_caps_lock_off ? DOT_TURNED_OFF_PERIOD : DOT_TURNED_ON_PERIOD;
        break;
    }
    default:
        break;
    }

    return timeout_period;
}

void update_status(void)
{
    if (get_sequence_state() == IDLE)
    {
        status = compose_status(DOT1, false);
    }
    else if (status == compose_status(DOT6, true))
    {
        status = compose_status(IDLE, true);
    }
    else
    {
        // either turn led off or go to the next sequence state and turn it on
        ++status;
    }
}
