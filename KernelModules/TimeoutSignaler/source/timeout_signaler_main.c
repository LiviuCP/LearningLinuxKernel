#include <linux/cdev.h>
#include <linux/ctype.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>

#include "timeout_signaler_impl.h"

#define SUCCESS 0
#define SUPPORTED_MINOR_NUMBERS_COUNT 1

// 9999 is an arbitrarily chosen "magic number" (in a "real" (production) system an official assignment would be
// required; might be the major driver number)
#define IOCTL_REQUEST_TIMEOUT_SIGNAL _IOW(9999, 'a', size_t*)
#define IOCTL_CANCEL_TIMEOUT_SIGNAL_REQUEST _IOW(9999, 'b', void*)

MODULE_LICENSE("GPL");

MODULE_DESCRIPTION("This module implements signalling the passing of a time period.\n"
                   "The requested time period (in milliseconds) is provided by user application via IOCTL.\n"
                   "The module sends a signal to the user process once the period has passed.\n"
                   "Cancelling from user side is also possible while the time period is being counted.\n");

MODULE_AUTHOR("Liviu Popa");

static struct class* timeout_signaler_class = NULL;
static struct cdev timeout_signaler_cdev;

static int major_number = 0;
static int is_device_open = 0;

static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);
static long device_ioctl(struct file*, unsigned int, unsigned long);

static struct file_operations file_ops = {
    .owner = THIS_MODULE, .open = device_open, .unlocked_ioctl = device_ioctl, .release = device_release};

static void do_module_cleanup(void); // destroy character device, delete device files,
                                     // delete class, unregister module

static int timeout_signaler_init(void)
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

        cdev_init(&timeout_signaler_cdev, &file_ops);

        int cdev_add_result = cdev_add(&timeout_signaler_cdev, major_number, SUPPORTED_MINOR_NUMBERS_COUNT);

        if (cdev_add_result < 0)
        {
            pr_alert("%s: cannot add device to the system\n", THIS_MODULE->name);
            do_module_cleanup();
            break;
        }

        timeout_signaler_class = class_create("timeout_signaler_class");

        if (!timeout_signaler_class)
        {
            pr_alert("%s: cannot create the struct class (timeout_signaler_class)\n", THIS_MODULE->name);
            do_module_cleanup();
            break;
        }

        if (!device_create(timeout_signaler_class, NULL, MKDEV(major_number, 0), NULL, "timeoutsignaler"))
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

static void timeout_signaler_exit(void)
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
    case IOCTL_REQUEST_TIMEOUT_SIGNAL:
        result = ioctl_request_timeout_signal(get_current(), (size_t*)arg);
        break;
    case IOCTL_CANCEL_TIMEOUT_SIGNAL_REQUEST:
        result = ioctl_cancel_timeout_signal_request(get_current());
        break;
    default:
        break;
    }

    return result;
}

static void do_module_cleanup()
{
    if (timeout_signaler_class)
    {
        device_destroy(timeout_signaler_class, MKDEV(major_number, 0));
        class_destroy(timeout_signaler_class);
    }

    cdev_del(&timeout_signaler_cdev);
    unregister_chrdev(major_number, THIS_MODULE->name);
}

module_init(timeout_signaler_init);
module_exit(timeout_signaler_exit);
