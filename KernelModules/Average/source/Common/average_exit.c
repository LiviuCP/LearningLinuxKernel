#include <linux/module.h>
#include <linux/moduleparam.h>

MODULE_LICENSE("GPL");

static char* exit_message = "";

module_param(exit_message, charp, S_IRUSR | S_IWUSR);
MODULE_PARM_DESC(exit_message, " an optional message to be sent upon average module unloading");

static void __exit average_exit(void)
{
    if (strlen(exit_message) > 0)
    {
        pr_info("%s: %s\n", THIS_MODULE->name, exit_message);
    }
    else
    {
        pr_info("%s: the average module exited!\n", THIS_MODULE->name);
    }
}

module_exit(average_exit);
