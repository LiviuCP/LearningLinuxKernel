#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/sysfs.h>

#include "division_impl.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This module divides two integers and provides the quotient and remainder. Four attributes are "
                   "being used: 2 are read/write (divided/divider) and 2 are read-only (quotient/remainder)\n");
MODULE_AUTHOR("Liviu Popa");

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
    const int result = store_divided_value(data, buf);

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
    const int result = store_divider_value(data, buf);

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

    store_command(data, buf);
    return count;
}

static void division_release(struct kobject* kobj)
{
    struct division_data* data = container_of(kobj, struct division_data, division_kobj);
    pr_info("%s: freeing data object that contains kobject \"%s\"\n", THIS_MODULE->name, kobj->name);
    kfree(data);
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
    .sysfs_ops = &kobj_sysfs_ops, .release = division_release, .default_groups = division_groups};

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
            result = init_data(data, divided, divider);
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
    pr_info("%s: putting the kobject: %s\n", THIS_MODULE->name, data->division_kobj.name);

    // this calls the release function (division_release()) for the data object containing the kobject
    kobject_put(&data->division_kobj);

    pr_info("%s: the module exited!\n", THIS_MODULE->name);
}

module_init(division_init);
module_exit(division_exit);
