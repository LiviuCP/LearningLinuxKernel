#include <linux/cdev.h>
#include <linux/ctype.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>

#include "ioctl_division_impl.h"

MODULE_LICENSE("GPL");

MODULE_DESCRIPTION("This module divides two integers and provides the quotient and remainder.\nAll operations are "
                   "implemented via IOCTL commands.\n");

MODULE_AUTHOR("Liviu Popa");

static struct class* ioctl_division_class = NULL;
static struct cdev ioctl_division_cdev;

static int major_number = 0;
static int is_device_open = 0;

static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);
static long device_ioctl(struct file*, unsigned int, unsigned long);

static struct file_operations file_ops = {
    .owner = THIS_MODULE, .open = device_open, .unlocked_ioctl = device_ioctl, .release = device_release};

static void do_module_cleanup(void); // destroy character device, delete device files,
                                     // delete class, unregister module

static int ioctl_division_init(void)
{
    int result = -1;

    do
    {
        major_number = register_chrdev(major_number, THIS_MODULE->name, &file_ops);

        if (major_number < 0)
        {
            pr_alert("%s: registering char device failed\n", THIS_MODULE->name);
            break;
        }

        cdev_init(&ioctl_division_cdev, &file_ops);

        int cdev_add_result = cdev_add(&ioctl_division_cdev, major_number, SUPPORTED_MINOR_NUMBERS_COUNT);

        if (cdev_add_result < 0)
        {
            pr_alert("%s: cannot add device to the system\n", THIS_MODULE->name);
            do_module_cleanup();
            break;
        }

        ioctl_division_class = class_create(THIS_MODULE, "ioctl_division_class");

        if (!ioctl_division_class)
        {
            pr_alert("%s: cannot create the struct class (ioctl_division_class)\n", THIS_MODULE->name);
            do_module_cleanup();
            break;
        }

        if (!device_create(ioctl_division_class, NULL, MKDEV(major_number, 0), NULL, "ioctldivision"))
        {
            pr_alert("%s: cannot create the device!\n", THIS_MODULE->name);
            do_module_cleanup();
            break;
        }

        result = SUCCESS;
        pr_info("%s: device registered, major number %d successfully assigned\n", THIS_MODULE->name, major_number);
    } while (false);

    return result;
}

static void ioctl_division_exit(void)
{
    do_module_cleanup();
    pr_info("%s: device with major number %d unregistered\n", THIS_MODULE->name, major_number);
}

static int device_open(struct inode* inode, struct file* file)
{
    int result = SUCCESS;

    if (!is_device_open)
    {
        pr_info("%s: opening device\n", THIS_MODULE->name);

        ++is_device_open;
        try_module_get(THIS_MODULE);
    }
    else
    {
        result = -EBUSY;
        pr_warn("%s: device is busy\n", THIS_MODULE->name);
    }

    return result;
}

static int device_release(struct inode* inode, struct file* file)
{
    pr_info("%s: releasing device\n", THIS_MODULE->name);

    --is_device_open;
    module_put(THIS_MODULE);

    return SUCCESS;
}

static long device_ioctl(struct file* file, unsigned int command, unsigned long arg)
{
    long result = 0;

    switch (command)
    {
    case IOCTL_STORE_DIVIDED_VALUE:
        result = ioctl_store_divided_value((int*)arg);
        break;
    case IOCTL_SHOW_DIVIDED_VALUE:
        result = ioctl_show_divided_value((int*)arg);
        break;
    case IOCTL_STORE_DIVIDER_VALUE:
        result = ioctl_store_divider_value((int*)arg);
        break;
    case IOCTL_SHOW_DIVIDER_VALUE:
        result = ioctl_show_divider_value((int*)arg);
        break;
    case IOCTL_SHOW_QUOTIENT_VALUE:
        result = ioctl_show_quotient_value((int*)arg);
        break;
    case IOCTL_SHOW_REMAINDER_VALUE:
        result = ioctl_show_remainder_value((int*)arg);
        break;
    case IOCTL_DIVIDE:
        result = ioctl_divide();
        break;
    case IOCTL_RESET:
        result = ioctl_reset();
        break;
    case IOCTL_GET_SYNCED_STATUS:
        result = ioctl_get_synced_status((uint8_t*)arg);
        break;
    default:
        break;
    }

    return result;
}

static void do_module_cleanup()
{
    if (ioctl_division_class)
    {
        device_destroy(ioctl_division_class, MKDEV(major_number, 0));
        class_destroy(ioctl_division_class);
    }

    cdev_del(&ioctl_division_cdev);
    unregister_chrdev(major_number, THIS_MODULE->name);
}

module_init(ioctl_division_init);
module_exit(ioctl_division_exit);
