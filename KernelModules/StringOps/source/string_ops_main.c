#include <linux/cdev.h>
#include <linux/ctype.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>

#include "string_ops_impl.h"

MODULE_LICENSE("GPL");

MODULE_DESCRIPTION(
    "This driver module performs various operations on strings depending on the minor version number.\n"
    "Four minor numbers are currently supported:\n"
    "- 0: read-only access, last provided user input can be retrieved\n"
    "- 1: user provided string is converted to lower-case\n"
    "- 2: user provided string is trimmed and reverted\n"
    "- 3: the length of the (trimmed) user provided string is calculated and appended to (trimmed) string\n"
    "Any other minor number is not supported and no operation will be performed.\n");

MODULE_AUTHOR("Liviu Popa");

static struct class* string_ops_class = NULL;
static struct cdev string_ops_cdev;

static int major_number = 0;
static int minor_number = -1;
static int is_device_open = 0;

static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);
static ssize_t device_read(struct file*, char*, size_t, loff_t*);
static ssize_t device_write(struct file*, const char*, size_t, loff_t*);

static struct file_operations file_ops = {
    .owner = THIS_MODULE, .read = device_read, .write = device_write, .open = device_open, .release = device_release};

static void do_module_cleanup(size_t existing_minor_numbers_count); // destroy character device, delete device files,
                                                                    // delete class, unregister module

static int string_ops_init(void)
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

        cdev_init(&string_ops_cdev, &file_ops);

        int cdev_add_result = cdev_add(&string_ops_cdev, major_number, SUPPORTED_MINOR_NUMBERS_COUNT);

        if (cdev_add_result < 0)
        {
            pr_alert("%s: cannot add device to the system\n", THIS_MODULE->name);
            do_module_cleanup(0);
            break;
        }

        string_ops_class = class_create("string_ops_class");

        if (!string_ops_class)
        {
            pr_alert("%s: cannot create the struct class (string_ops_class)\n", THIS_MODULE->name);
            do_module_cleanup(0);
            break;
        }

        int device_minor_number = 0;

        for (; device_minor_number < SUPPORTED_MINOR_NUMBERS_COUNT; ++device_minor_number)
        {
            if (!device_create(string_ops_class, NULL, MKDEV(major_number, device_minor_number), NULL, "stringops%d",
                               device_minor_number))
            {
                pr_alert("%s: cannot create the device with minor %d!\n", THIS_MODULE->name, device_minor_number);
                break;
            }
        }

        if (device_minor_number != SUPPORTED_MINOR_NUMBERS_COUNT)
        {
            // destroy all previously created minor number devices (current number creation failed)
            const size_t existing_minor_numbers_count = (size_t)device_minor_number;
            do_module_cleanup((size_t)existing_minor_numbers_count);
            break;
        }

        result = SUCCESS;
        reset_module_data();
        pr_info("%s: device registered, major number %d successfully assigned\n", THIS_MODULE->name, major_number);
    } while (false);

    return result;
}

static void string_ops_exit(void)
{
    do_module_cleanup(SUPPORTED_MINOR_NUMBERS_COUNT);
    pr_info("%s: device with major number %d unregistered\n", THIS_MODULE->name, major_number);
}

static int device_open(struct inode* inode, struct file* file)
{
    int result = -1;

    if (!is_device_open)
    {
        minor_number = iminor(inode);

        pr_info("%s: opening device, minor number is: %d\n", THIS_MODULE->name, minor_number);

        if (is_valid_minor_number(minor_number))
        {
            result = SUCCESS;

            link_minor_number_data(minor_number);
            ++is_device_open;
            try_module_get(THIS_MODULE);
        }
        else
        {
            pr_alert("%s: minor number %d is not supported!\n", THIS_MODULE->name, minor_number);
        }
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
    pr_info("%s: releasing device, minor number is: %d\n", THIS_MODULE->name, minor_number);

    --is_device_open;
    module_put(THIS_MODULE);

    return SUCCESS;
}

static ssize_t device_read(struct file* filp, char* buffer, size_t length, loff_t* offset)
{
    return device_read_impl(filp, buffer, length, offset, minor_number);
}

static ssize_t device_write(struct file* filp, const char* buffer, size_t length, loff_t* offset)
{
    return device_write_impl(filp, buffer, length, offset, minor_number);
}

static void do_module_cleanup(size_t existing_minor_numbers_count)
{
    if (string_ops_class)
    {
        for (int device_minor_number = 0; (size_t)device_minor_number < existing_minor_numbers_count;
             ++device_minor_number)
        {
            device_destroy(string_ops_class, MKDEV(major_number, device_minor_number));
        }

        class_destroy(string_ops_class);
    }

    cdev_del(&string_ops_cdev);
    unregister_chrdev(major_number, THIS_MODULE->name);
}

module_init(string_ops_init);
module_exit(string_ops_exit);
