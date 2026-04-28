#include <linux/cdev.h>
#include <linux/fs.h>

#include "ioctl_string_ops_impl.h"

#define SUCCESS 0
#define SUPPORTED_MINOR_NUMBERS_COUNT 1

// 9999 is an arbitrarily chosen "magic number" (in a "real" (production) system an official assignment would be
// required; might be the major driver number)
#define IOCTL_DO_MODULE_RESET _IOW(9999, 'a', void*)
#define IOCTL_IS_MODULE_RESET _IOR(9999, 'b', uint8_t*)
#define IOCTL_TRIM_USER_INPUT _IOW(9999, 'c', uint8_t*)
#define IOCTL_GET_CHARS_COUNT_FROM_BUFFER _IOR(9999, 'd', size_t*)
#define IOCTL_SET_OUTPUT_PREFIX _IOW(9999, 'e', void*)
#define IOCTL_GET_OUTPUT_PREFIX_SIZE _IOR(9999, 'f', size_t*)

MODULE_LICENSE("GPL");

MODULE_DESCRIPTION("This driver module provides and receives strings to/from user.\n"
                   "This happens by reading from or writing to the device file.\n"
                   "However the input/output can be altered by using various ioctl commands.\n");

MODULE_AUTHOR("Liviu Popa");

static struct class* ioctl_string_ops_class = NULL;
static struct cdev ioctl_string_ops_cdev;

static int major_number = 0;
static int is_device_open = 0;

static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);
static ssize_t device_read(struct file*, char*, size_t, loff_t*);
static ssize_t device_write(struct file*, const char*, size_t, loff_t*);
static long device_ioctl(struct file*, unsigned int, unsigned long);

static struct file_operations file_ops = {.owner = THIS_MODULE,
                                          .read = device_read,
                                          .write = device_write,
                                          .open = device_open,
                                          .unlocked_ioctl = device_ioctl,
                                          .release = device_release};

static void do_module_cleanup(void); // destroy character device, delete device files,
                                     // delete class, unregister module

static int ioctl_string_ops_init(void)
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

        cdev_init(&ioctl_string_ops_cdev, &file_ops);

        int cdev_add_result = cdev_add(&ioctl_string_ops_cdev, major_number, SUPPORTED_MINOR_NUMBERS_COUNT);

        if (cdev_add_result < 0)
        {
            pr_alert("%s: cannot add device to the system\n", THIS_MODULE->name);
            do_module_cleanup();
            break;
        }

        ioctl_string_ops_class = class_create("ioctl_string_ops_class");

        if (!ioctl_string_ops_class)
        {
            pr_alert("%s: cannot create the struct class (ioctl_string_ops_class)\n", THIS_MODULE->name);
            do_module_cleanup();
            break;
        }

        if (!device_create(ioctl_string_ops_class, NULL, MKDEV(major_number, 0), NULL, "ioctlstringops"))
        {
            pr_alert("%s: cannot create the device!\n", THIS_MODULE->name);
            do_module_cleanup();
            break;
        }

        result = SUCCESS;
        reset_buffers();
        pr_info("%s: device registered, major number %d successfully assigned\n", THIS_MODULE->name, major_number);
    } while (false);

    return result;
}

static void ioctl_string_ops_exit(void)
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

static ssize_t device_read(struct file* filp, char* buf, size_t length, loff_t* offset)
{
    return device_read_impl(filp, buf, length, offset);
}

static ssize_t device_write(struct file* filp, const char* buf, size_t length, loff_t* offset)
{
    return device_write_impl(filp, buf, length, offset);
}

static long device_ioctl(struct file* file, unsigned int command, unsigned long arg)
{
    long result = 0;

    switch (command)
    {
    case IOCTL_DO_MODULE_RESET: {
        result = ioctl_do_module_reset();
        break;
    }
    case IOCTL_IS_MODULE_RESET: {
        result = ioctl_is_module_reset((uint8_t*)arg);
        break;
    }
    case IOCTL_TRIM_USER_INPUT: {
        result = ioctl_trim_user_input((uint8_t*)arg);
        break;
    }
    case IOCTL_GET_CHARS_COUNT_FROM_BUFFER: {
        result = ioctl_get_chars_count_from_buffer((size_t*)arg);
        break;
    }
    case IOCTL_SET_OUTPUT_PREFIX: {
        result = ioctl_set_output_prefix((void*)arg);
        break;
    }
    case IOCTL_GET_OUTPUT_PREFIX_SIZE: {
        result = ioctl_get_output_prefix_size((size_t*)arg);
        break;
    }
    default:
        break;
    }

    return result;
}

static void do_module_cleanup()
{
    if (ioctl_string_ops_class)
    {
        device_destroy(ioctl_string_ops_class, MKDEV(major_number, 0));
        class_destroy(ioctl_string_ops_class);
    }

    cdev_del(&ioctl_string_ops_cdev);
    unregister_chrdev(major_number, THIS_MODULE->name);
}

module_init(ioctl_string_ops_init);
module_exit(ioctl_string_ops_exit);
