#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/sysfs.h>

#define EDIVBYZERO 1
#define MAX_COMMAND_STR_LENGTH 32
#define MAX_STATUS_STR_LENGTH 16

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This module divides two integers and provides the quotient and remainder. Four attributes are "
                   "being used: 2 are read/write (divided/divider) and 2 are read-only (quotient/remainder)\n");
MODULE_AUTHOR("Liviu Popa");

static const char* divide_cmd_str = "divide"; // newline included, this is how the command is read from buffer
static const char* reset_cmd_str = "reset";   // same here
static const char* dirty_status_str = "dirty";
static const char* synced_status_str = "synced";

static const size_t divide_cmd_str_length = 6;
static const size_t reset_cmd_str_length = 5;

/* There are two valid commands:
   - "divide": calculates quotient and remainder based on existing divided/divider
   - "reset": resets divided/divider to initial values

   There are two possible statuses:
   - "dirty": divided/divider have been modified but quotient/remainder haven't been (successfully) recalculated
   - "synced": quotient and remainder have been calculated based on existing divided and divider (all four values are
   "in sync")

   Note: when an invalid command is issued or the command encountered an error the status remains unchanged.
*/

struct division_data
{
    struct kobject division_kobj;
    int divided;
    int divider;
    int quotient;
    int remainder;
    char command[MAX_COMMAND_STR_LENGTH];
    char status[MAX_STATUS_STR_LENGTH];
};

static int compute_quotient_and_remainder(struct division_data* data)
{
    int result = 0;

    if (data)
    {
        if (data->divider != 0)
        {
            data->quotient = data->divided / data->divider;
            data->remainder = data->divided % data->divider;
            memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
            strncpy(data->status, synced_status_str, strlen(synced_status_str));

            pr_info("%s: divided: %d\n", THIS_MODULE->name, data->divided);
            pr_info("%s: divider: %d\n", THIS_MODULE->name, data->divider);
            pr_info("%s: quotient: %d\n", THIS_MODULE->name, data->quotient);
            pr_info("%s: remainder: %d\n", THIS_MODULE->name, data->remainder);
        }
        else
        {
            result = -EDIVBYZERO;
            pr_err("%s: cannot perform operation (division by 0)!\n", THIS_MODULE->name);
        }
    }
    else
    {
        pr_warn("%s: NULL data object!\n", THIS_MODULE->name);
    }

    return result;
}

/* VARIABLES AND PARAMETERS */

static int divided = 0;
static int divider = 1;

module_param(divided, int, S_IRUSR | S_IWUSR);
module_param(divider, int, S_IRUSR | S_IWUSR);

struct division_data* data = NULL;

/* SYSFS access methods for attributes */

static ssize_t divided_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    struct division_data* data = container_of(kobj, struct division_data, division_kobj);
    return sysfs_emit(buf, "%d\n", data->divided);
}

static ssize_t divided_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    struct division_data* data = container_of(kobj, struct division_data, division_kobj);
    const int result = kstrtoint(buf, 10, &data->divided);

    if (result >= 0)
    {
        memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
        strncpy(data->status, dirty_status_str, strlen(dirty_status_str));
        pr_info("%s: new divided value: %d\n", THIS_MODULE->name, data->divided);
    }

    return result < 0 ? 0 : count;
}

static ssize_t divider_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    struct division_data* data = container_of(kobj, struct division_data, division_kobj);
    return sysfs_emit(buf, "%d\n", data->divider);
}

static ssize_t divider_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    struct division_data* data = container_of(kobj, struct division_data, division_kobj);
    const int result = kstrtoint(buf, 10, &data->divider);

    if (result >= 0)
    {
        memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
        strncpy(data->status, dirty_status_str, strlen(dirty_status_str));
        pr_info("%s: new divider value: %d\n", THIS_MODULE->name, data->divider);
    }

    return result < 0 ? 0 : count;
}

// no store to be defined here as the quotient is read-only
static ssize_t quotient_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    struct division_data* data = container_of(kobj, struct division_data, division_kobj);
    return sysfs_emit(buf, "%d\n", data->quotient);
}

// same here
static ssize_t remainder_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    struct division_data* data = container_of(kobj, struct division_data, division_kobj);
    return sysfs_emit(buf, "%d\n", data->remainder);
}

// no show to be defined here as the command is write-only
static ssize_t command_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    struct division_data* data = container_of(kobj, struct division_data, division_kobj);

    memset(data->command, '\0', MAX_COMMAND_STR_LENGTH);
    strncpy(data->command, buf, MAX_COMMAND_STR_LENGTH - 1);

    size_t command_length = strlen(data->command);

    // TODO: create trimming function
    if (command_length > 0 && data->command[command_length - 1] == '\n')
    {
        data->command[command_length - 1] = '\0';
        --command_length;
    }

    pr_info("%s: issued command: %s\n", THIS_MODULE->name, data->command);

    if (command_length == divide_cmd_str_length && strncmp(data->command, divide_cmd_str, divide_cmd_str_length) == 0)
    {
        pr_info("%s: computing new quotient and remainder\n", THIS_MODULE->name);
        compute_quotient_and_remainder(data);
    }
    else if (command_length == reset_cmd_str_length && strncmp(data->command, reset_cmd_str, reset_cmd_str_length) == 0)
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

    return count;
}

// no store to be defined here as the status is read-only
static ssize_t status_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    struct division_data* data = container_of(kobj, struct division_data, division_kobj);
    return sysfs_emit(buf, "%s\n", data->status);
}

/* SYSFS attributes */

static struct kobj_attribute divided_attribute = __ATTR(divided, 0600, divided_show, divided_store);
static struct kobj_attribute divider_attribute = __ATTR(divider, 0600, divider_show, divider_store);
static struct kobj_attribute quotient_attribute = __ATTR(quotient, 0400, quotient_show, NULL);
static struct kobj_attribute remainder_attribute = __ATTR(remainder, 0400, remainder_show, NULL);
static struct kobj_attribute command_attribute = __ATTR(command, 0200, NULL, command_store);
static struct kobj_attribute status_attribute = __ATTR(status, 0400, status_show, NULL);

static struct attribute* division_attrs[] = {&divided_attribute.attr,
                                             &divider_attribute.attr,
                                             &quotient_attribute.attr,
                                             &remainder_attribute.attr,
                                             &command_attribute.attr,
                                             &status_attribute.attr,
                                             NULL};

// these statements can be replaced by: "ATTRIBUTE_GROUPS(division);" (see sysfs.h)
// I preferred to keep the unwrapped for better readability
static const struct attribute_group division_group = {.attrs = division_attrs};
static const struct attribute_group* division_groups[] = {&division_group, NULL};

// kobj_sysfs_ops is the default sysfs_ops (binds all access functions defined above into division_ktype)
static struct kobj_type division_ktype = {
    .sysfs_ops = &kobj_sysfs_ops, .release = NULL, .default_groups = division_groups};

/* INIT/EXIT */

static int division_init(void)
{
    // resulting directory structure: "/sys/kernel/division"
    // - kernel_kobj (parent kobject) => "/sys/kernel"
    // - division_kobj_name => "/division"
    const char* division_kobj_name = "division";

    int result = 0;

    pr_info("%s: initializing module\n", THIS_MODULE->name);
    data = kzalloc(sizeof(struct division_data), GFP_KERNEL);

    if (data)
    {
        result = kobject_init_and_add(&data->division_kobj, &division_ktype, kernel_kobj, "%s", division_kobj_name);

        if (result == 0)
        {
            data->divided = divided;
            data->divider = divider;
            memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
            strncpy(data->status, dirty_status_str, strlen(dirty_status_str));

            pr_info("%s: computing quotient and remainder\n", THIS_MODULE->name);
            result = compute_quotient_and_remainder(data);
        }
        else
        {
            result = -ENOMEM;
        }
    }
    else
    {
        result = -ENOMEM;
    }

    if (result == -ENOMEM)
    {
        pr_err("%s: unable to initialize sysfs object \"%s\" (no memory)!\n", THIS_MODULE->name, division_kobj_name);
    }

    return result;
}

static void division_exit(void)
{
    kobject_put(&data->division_kobj);
    kfree(data);
    pr_info("%s: the module exited!\n", THIS_MODULE->name);
}

module_init(division_init);
module_exit(division_exit);
