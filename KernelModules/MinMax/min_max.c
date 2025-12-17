#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This module retrieves the min/max value of two integers!\n");
MODULE_AUTHOR("Liviu Popa");

static int first = 0;
static int second = 0;

module_param(first, int, S_IRUSR | S_IWUSR);
module_param(second, int, S_IRUSR | S_IWUSR);

static int get_min(void)
{
    return first < second ? first : second;
}

static int get_max(void)
{
    return first > second ? first : second;
}

static int min_max_init(void)
{
    pr_info("%s: initializing module\n", THIS_MODULE->name);
    pr_info("%s: the values of the parameters are: %d and %d\n", THIS_MODULE->name, first, second);
    pr_info("%s: the min value is: %d\n", THIS_MODULE->name, get_min());
    pr_info("%s: the max value is: %d\n", THIS_MODULE->name, get_max());

    return 0;
}

static void min_max_exit(void)
{
    pr_info("%s: the module exited!\n", THIS_MODULE->name);
}

module_init(min_max_init);
module_exit(min_max_exit);
