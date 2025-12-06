#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/string.h>
#include <linux/sysfs.h>

#define EDIVBYZERO 1
#define EREADONLY 2

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This module divides two integers and provides the quotient and remainder. Four attributes are "
                   "being used: 2 are read/write (divided/divider) and 2 are read-only (quotient/remainder)\n");
MODULE_AUTHOR("Liviu Popa");

/* VARIABLES */

static int divided = 0;
static int divider = 1;
static int quotient = 0;
static int remainder = 0;

static int compute_quotient_and_remainder(void)
{
    int result = 0;

    if (divider != 0)
    {
        quotient = divided / divider;
        remainder = divided % divider;

        pr_info("%s: divided: %d\n", THIS_MODULE->name, divided);
        pr_info("%s: divider: %d\n", THIS_MODULE->name, divider);
        pr_info("%s: quotient: %d\n", THIS_MODULE->name, quotient);
        pr_info("%s: remainder: %d\n", THIS_MODULE->name, remainder);
    }
    else
    {
        result = -EDIVBYZERO;
        pr_err("%s: cannot perform operation (division by 0)!\n", THIS_MODULE->name);
    }

    return result;
}

/* PARAMETERS */

module_param(divided, int, S_IRUSR | S_IWUSR);
module_param(divider, int, S_IRUSR | S_IWUSR);

/* SYSFS objects/attributes */

struct kobject division_kobj;

static struct attribute divided_attribute = {.name = "divided", .mode = 0600};
static struct attribute divider_attribute = {.name = "divider", .mode = 0600};
static struct attribute quotient_attribute = {.name = "quotient", .mode = 0400};
static struct attribute remainder_attribute = {.name = "remainder", .mode = 0400};

static struct attribute* division_attrs[] = {&divided_attribute, &divider_attribute, &quotient_attribute,
                                             &remainder_attribute, NULL};

// these statements can be replaced by: "ATTRIBUTE_GROUPS(division);" (see sysfs.h)
// I preferred to keep the unwrapped for better readability
static const struct attribute_group division_group = {.attrs = division_attrs};
static const struct attribute_group* division_groups[] = {&division_group, NULL};

static ssize_t division_show(struct kobject* kobj, struct attribute* attr, char* buf)
{
    int result = 0;

    if (strncmp(attr->name, "divided", 7) == 0)
    {
        result = sysfs_emit(buf, "%d\n", divided);
    }
    else if (strncmp(attr->name, "divider", 7) == 0)
    {
        result = sysfs_emit(buf, "%d\n", divider);
    }
    else if (strncmp(attr->name, "quotient", 8) == 0)
    {
        result = sysfs_emit(buf, "%d\n", quotient);
    }
    else
    {
        result = sysfs_emit(buf, "%d\n", remainder);
    }

    return result;
}

static ssize_t division_store(struct kobject* kobj, struct attribute* attr, const char* buf, size_t count)
{
    int result = -EREADONLY;

    if (strncmp(attr->name, "divided", 7) == 0)
    {
        pr_info("%s: new divided value, recalculating quotient and remainder\n", THIS_MODULE->name);
        result = kstrtoint(buf, 10, &divided);
    }
    else if (strncmp(attr->name, "divider", 7) == 0)
    {
        pr_info("%s: new divider value, recalculating quotient and remainder\n", THIS_MODULE->name);
        result = kstrtoint(buf, 10, &divider);
    }
    else
    {
        pr_err("%s: attribute %s is read-only!\n", THIS_MODULE->name, attr->name);
    }

    if (result == 0)
    {
        result = compute_quotient_and_remainder();
    }

    return result < 0 ? result : count;
}

struct sysfs_ops division_ops = {.show = division_show, .store = division_store};

static struct kobj_type division_ktype = {
    .sysfs_ops = &division_ops, .release = NULL, .default_groups = division_groups};

/* INIT/EXIT */

static int division_init(void)
{
    pr_info("%s: initializing module\n", THIS_MODULE->name);
    pr_info("%s: doing calculation\n", THIS_MODULE->name);

    int result = compute_quotient_and_remainder();

    if (result == 0)
    {
        // resulting directory structure: "/sys/kernel/division"
        // - kernel_kobj (parent kobject) => "/sys/kernel"
        // - division_kobj_name => "/division"
        const char* division_kobj_name = "division";
        result = kobject_init_and_add(&division_kobj, &division_ktype, kernel_kobj, "%s", division_kobj_name);

        if (result != 0)
        {
            pr_err("%s: unable to initialize sysfs object \"%s\" (no memory)!\n", THIS_MODULE->name,
                   division_kobj_name);
            result = -ENOMEM;
        }
    }

    return result;
}

static void division_exit(void)
{
    kobject_put(&division_kobj);
    pr_info("%s: the module exited!\n", THIS_MODULE->name);
}

module_init(division_init);
module_exit(division_exit);
