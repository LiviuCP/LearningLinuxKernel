#include <linux/ctype.h>
#include <linux/module.h>

#include "division_impl.h"

MODULE_LICENSE("GPL");

#define EDIVBYZERO 1
#define ENULLDATAOBJECT 2
#define EOTHERNULLOBJECT 3

static const char* divide_cmd_str = "divide";
static const char* reset_cmd_str = "reset";
static const char* dirty_status_str = "dirty";
static const char* synced_status_str = "synced";

static const size_t divide_cmd_str_length = 6;
static const size_t reset_cmd_str_length = 5;

extern void trim_and_copy_string(char* dest, const char* src, size_t max_str_length);

static int compute_quotient_and_remainder(struct division_data* data)
{
    int result = 0;

    do
    {
        if (!data)
        {
            result = -ENULLDATAOBJECT;
            pr_warn("%s: NULL data object!\n", THIS_MODULE->name);
            break;
        }

        if (data->divider == 0)
        {
            result = -EDIVBYZERO;
            pr_err("%s: cannot perform operation (division by 0)!\n", THIS_MODULE->name);
            break;
        }

        data->quotient = data->divided / data->divider;
        data->remainder = data->divided % data->divider;
        memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
        strncpy(data->status, synced_status_str, strlen(synced_status_str));

        pr_info("%s: divided: %d\n", THIS_MODULE->name, data->divided);
        pr_info("%s: divider: %d\n", THIS_MODULE->name, data->divider);
        pr_info("%s: quotient: %d\n", THIS_MODULE->name, data->quotient);
        pr_info("%s: remainder: %d\n", THIS_MODULE->name, data->remainder);
    } while (false);

    return result;
}

int init_data(struct division_data* data, int divided, int divider)
{
    int result = 0;

    if (data)
    {
        data->divided = divided;
        data->divider = divider;
        memset(data->command, '\0', MAX_COMMAND_STR_LENGTH);
        memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
        strncpy(data->status, dirty_status_str, strlen(dirty_status_str));

        pr_info("%s: computing quotient and remainder\n", THIS_MODULE->name);

        result = compute_quotient_and_remainder(data);
    }
    else
    {
        result = -ENULLDATAOBJECT;
        pr_warn("%s: NULL data object!\n", THIS_MODULE->name);
    }

    return result;
}

int store_divided_value(struct division_data* data, const char* divided_str)
{
    int result = 0;

    do
    {
        if (!data)
        {
            result = -ENULLDATAOBJECT;
            pr_warn("%s: NULL data object!\n", THIS_MODULE->name);
            break;
        }

        if (!divided_str)
        {
            result = -EOTHERNULLOBJECT;
            pr_warn("%s: NULL divided string!\n", THIS_MODULE->name);
            break;
        }

        result = kstrtoint(divided_str, 10, &data->divided);

        if (result >= 0)
        {
            memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
            strncpy(data->status, dirty_status_str, strlen(dirty_status_str));
            pr_info("%s: new divided value: %d\n", THIS_MODULE->name, data->divided);
        }
    } while (false);

    return result;
}

int store_divider_value(struct division_data* data, const char* divider_str)
{
    int result = 0;

    do
    {
        if (!data)
        {
            result = -ENULLDATAOBJECT;
            pr_warn("%s: NULL data object!\n", THIS_MODULE->name);
            break;
        }

        if (!divider_str)
        {
            result = -EOTHERNULLOBJECT;
            pr_warn("%s: NULL divider string!\n", THIS_MODULE->name);
            break;
        }

        result = kstrtoint(divider_str, 10, &data->divider);

        if (result >= 0)
        {
            memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
            strncpy(data->status, dirty_status_str, strlen(dirty_status_str));
            pr_info("%s: new divider value: %d\n", THIS_MODULE->name, data->divider);
        }
    } while (false);

    return result;
}

/* There are two valid commands:
   - "divide": calculates quotient and remainder based on existing divided/divider
   - "reset": resets divided/divider to initial values

   There are two possible statuses:
   - "dirty": divided/divider have been modified but quotient/remainder haven't been (successfully) recalculated
   - "synced": quotient and remainder have been calculated based on existing divided and divider (all four values are
   "in sync")

   Note: when an invalid command is issued or the command encountered an error the status remains unchanged.
*/

void store_command(struct division_data* data, const char* command_str)
{
    do
    {
        if (!data)
        {
            pr_warn("%s: NULL data object!\n", THIS_MODULE->name);
            break;
        }

        if (!command_str)
        {
            pr_warn("%s: NULL command string!\n", THIS_MODULE->name);
            break;
        }

        trim_and_copy_string(data->command, command_str, MAX_COMMAND_STR_LENGTH);
        const size_t command_length = strlen(data->command);

        pr_info("%s: issued command: %s\n", THIS_MODULE->name, data->command);

        if (command_length == divide_cmd_str_length &&
            strncmp(data->command, divide_cmd_str, divide_cmd_str_length) == 0)
        {
            pr_info("%s: computing new quotient and remainder\n", THIS_MODULE->name);
            compute_quotient_and_remainder(data);
        }
        else if (command_length == reset_cmd_str_length &&
                 strncmp(data->command, reset_cmd_str, reset_cmd_str_length) == 0)
        {
            pr_info("%s: resetting quotient and remainder\n", THIS_MODULE->name);
            data->divided = 0;
            data->divider = 1;
            compute_quotient_and_remainder(data);
        }
        else
        {
            pr_warn("%s: invalid command: %s\n", THIS_MODULE->name, data->command);
        }
    } while (false);
}
