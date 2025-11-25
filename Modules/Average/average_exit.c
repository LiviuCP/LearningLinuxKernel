#include <linux/module.h>
#include <linux/moduleparam.h>

MODULE_LICENSE("GPL");

static char *exitMessage = "";

extern int is_empty_string(const char *);

module_param(exitMessage, charp, S_IRUSR | S_IWUSR);
MODULE_PARM_DESC(exitMessage, " an optional message to be sent upon average module unloading");

static void __exit average_exit(void)
{
    if (!is_empty_string(exitMessage))
    {
        pr_info("%s: %s\n", THIS_MODULE->name, exitMessage);
    }
    else
    {
        pr_info("%s: the average module exited!\n", THIS_MODULE->name);
    }
}

module_exit(average_exit);
