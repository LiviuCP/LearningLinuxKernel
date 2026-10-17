#include <linux/cdev.h>
#include <linux/fs.h>

#include "wait_queue_impl.h"

#define SUCCESS 0
#define SUPPORTED_MINOR_NUMBERS_COUNT 1

MODULE_LICENSE("GPL");

MODULE_DESCRIPTION(
    "This driver module implements a waiting queue.\n"
    "The user enters a string of characters which are appended to the input buffer content (if any).\n"
    "The operation can be repeated until a characters count threshold has been reached.\n"
    "When this happens a conversion (string formatting) is performed and written into the output buffer.\n"
    "If the output buffer is not empty the conversion gets deferred until the user reads its content.\n"
    "The user can continue to enter input chars until the input buffer gets full.\n"
    "A waiting queue intermediates the conversion by keeping track of both conditions: reaching the input threshold "
    "and having an empty output buffer.\n"
    "The input buffer gets cleared once the formatting has been performed.\n"
    "The output buffer gets cleared once the user has read the output.\n");

MODULE_AUTHOR("Liviu Popa");

static struct class* wait_queue_class = NULL;
static struct cdev wait_queue_cdev;

static int major_number = 0;
static int is_device_open = 0;

static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);
static ssize_t device_read(struct file*, char*, size_t, loff_t*);
static ssize_t device_write(struct file*, const char*, size_t, loff_t*);

static struct file_operations file_ops = {
    .owner = THIS_MODULE, .read = device_read, .write = device_write, .open = device_open, .release = device_release};

static void do_module_cleanup(void); // destroy character device, delete device files,
                                     // delete class, unregister module

static int wait_queue_init(void)
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

        cdev_init(&wait_queue_cdev, &file_ops);

        int cdev_add_result = cdev_add(&wait_queue_cdev, major_number, SUPPORTED_MINOR_NUMBERS_COUNT);

        if (cdev_add_result < 0)
        {
            pr_alert("%s: cannot add device to the system\n", THIS_MODULE->name);
            do_module_cleanup();
            break;
        }

        wait_queue_class = class_create("wait_queue_class");

        if (!wait_queue_class)
        {
            pr_alert("%s: cannot create the struct class (wait_queue_class)\n", THIS_MODULE->name);
            do_module_cleanup();
            break;
        }

        if (!device_create(wait_queue_class, NULL, MKDEV(major_number, 0), NULL, "waitqueue"))
        {
            pr_alert("%s: cannot create the device!\n", THIS_MODULE->name);
            do_module_cleanup();
            break;
        }

        if (!init_queue())
        {
            pr_alert("%s: unable to initialize the queue\n", THIS_MODULE->name);
            do_module_cleanup();
            break;
        }

        result = SUCCESS;
        reset_module_data();
        pr_info("%s: device registered, major number %d successfully assigned\n", THIS_MODULE->name, major_number);
    } while (false);

    return result;
}

static void wait_queue_exit(void)
{
    clear_queue();
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

    handle_post_device_release_actions();

    return SUCCESS;
}

static ssize_t device_read(struct file* filp, char* buf, size_t length, loff_t* offset)
{
    return device_read_impl(filp, buf, length, offset);
}

static ssize_t device_write(struct file* filp, const char* buf, size_t length, loff_t* offset)
{
    return device_write_impl(filp, buf, length, offset);
}

static void do_module_cleanup()
{
    if (wait_queue_class)
    {
        device_destroy(wait_queue_class, MKDEV(major_number, 0));
        class_destroy(wait_queue_class);
    }

    cdev_del(&wait_queue_cdev);
    unregister_chrdev(major_number, THIS_MODULE->name);
}

module_init(wait_queue_init);
module_exit(wait_queue_exit);
