#include <linux/module.h>
#include <linux/uaccess.h>

#include "ioctl_division_impl.h"

static int divided = 0;
static int divider = 1;
static int quotient = 0;
static int remainder = 0;
static uint8_t is_synced = 1;

long ioctl_store_divided_value(const int* divided_value)
{
    long result = -1;

    if (divided_value)
    {
        int temp;
        const size_t bytes_not_copied_count = copy_from_user(&temp, divided_value, sizeof(int));

        if (bytes_not_copied_count == 0)
        {
            divided = temp;
            is_synced = 0;
            result = 0;
        }
        else
        {
            pr_err("%s: IOCTL: failed storing the divided!\n", THIS_MODULE->name);
        }
    }

    return result;
}

long ioctl_show_divided_value(int* divided_value)
{
    long result = -1;

    if (divided_value)
    {
        const size_t bytes_not_copied_count = copy_to_user(divided_value, &divided, sizeof(int));

        if (bytes_not_copied_count == 0)
        {
            result = 0;
        }
        else
        {
            pr_err("%s: IOCTL: failed showing the divided!\n", THIS_MODULE->name);
        }
    }

    return result;
}

long ioctl_store_divider_value(const int* divider_value)
{
    long result = -1;

    if (divider_value)
    {
        int temp;
        const size_t bytes_not_copied_count = copy_from_user(&temp, divider_value, sizeof(int));

        if (bytes_not_copied_count == 0)
        {
            divider = temp;
            is_synced = 0;
            result = 0;
        }
        else
        {
            pr_err("%s: IOCTL: failed storing the divider!\n", THIS_MODULE->name);
        }
    }

    return result;
}

long ioctl_show_divider_value(int* divider_value)
{
    long result = -1;

    if (divider_value)
    {
        const size_t bytes_not_copied_count = copy_to_user(divider_value, &divider, sizeof(int));

        if (bytes_not_copied_count == 0)
        {
            result = 0;
        }
        else
        {
            pr_err("%s: IOCTL: failed showing the divider!\n", THIS_MODULE->name);
        }
    }

    return result;
}

long ioctl_show_quotient_value(int* quotient_value)
{
    long result = -1;

    if (quotient_value)
    {
        const size_t bytes_not_copied_count = copy_to_user(quotient_value, &quotient, sizeof(int));

        if (bytes_not_copied_count == 0)
        {
            result = 0;
        }
        else
        {
            pr_err("%s: IOCTL: failed showing the quotient!\n", THIS_MODULE->name);
        }
    }

    return result;
}

long ioctl_show_remainder_value(int* remainder_value)
{
    long result = -1;

    if (remainder_value)
    {
        const size_t bytes_not_copied_count = copy_to_user(remainder_value, &remainder, sizeof(int));

        if (bytes_not_copied_count == 0)
        {
            result = 0;
        }
        else
        {
            pr_err("%s: IOCTL: failed showing the remainder!\n", THIS_MODULE->name);
        }
    }

    return result;
}

long ioctl_divide(void)
{
    long result = -1;

    if (divider != 0)
    {
        quotient = divided / divider;
        remainder = divided % divider;
        is_synced = 1;
        result = 0;
    }
    else
    {
        pr_err("%s: IOCTL: division by 0!\n", THIS_MODULE->name);
    }

    return result;
}

long ioctl_reset(void)
{
    divided = 0;
    divider = 1;
    quotient = 0;
    remainder = 0;
    is_synced = 1;

    return 0;
}

long ioctl_get_synced_status(uint8_t* is_synced_value)
{
    long result = -1;

    if (is_synced_value)
    {
        const size_t bytes_not_copied_count = copy_to_user(is_synced_value, &is_synced, sizeof(uint8_t));

        if (bytes_not_copied_count == 0)
        {
            result = 0;
        }
        else
        {
            pr_err("%s: IOCTL: failed showing the synced status!\n", THIS_MODULE->name);
        }
    }

    return result;
}
