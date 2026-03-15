#include <linux/cdev.h>
#include <linux/ctype.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>

#include "ioctl_string_ops_impl.h"

#define GET_BUFFER_SIZE _IOR(9999, 'a', size_t*)
#define TRIM_USER_INPUT _IOW(9999, 'b', uint8_t*)
#define DO_MODULE_RESET _IOW(9999, 'c', void*)
#define IS_MODULE_RESET _IOR(9999, 'd', uint8_t*)
#define SET_OUTPUT_PREFIX _IOW(9999, 'e', void*)

MODULE_LICENSE("GPL");

MODULE_DESCRIPTION("This driver module provides and receives strings to/from user.\n"
                   "This happens by reading from or writing to the device file.\n"
                   "However the input/output can be altered by using various ioctl commands.\n");

MODULE_AUTHOR("Liviu Popa");

static struct class* string_ops_class = NULL;
static struct cdev string_ops_cdev;

static int major_number = 0;
static int is_device_open = 0;

static char buffer[BUFFER_SIZE]; // shared input/output buffer
static char* buffer_ptr;         // points to the input/output buffer
static char output_prefix[BUFFER_SIZE];
static char* output_prefix_ptr;

/* 1-6: reserved, 0 - trim user input (default true) */
static uint8_t settings = DEFAULT_SETTINGS;

extern void trim_and_copy_string(char* dest, const char* src, size_t max_str_length);

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

static void reset_buffers(void);     // input/output buffers reset
static void do_module_cleanup(void); // destroy character device, delete device files,
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
            do_module_cleanup();
            break;
        }

        string_ops_class = class_create(THIS_MODULE, "string_ops_class");

        if (!string_ops_class)
        {
            pr_alert("%s: cannot create the struct class (string_ops_class)\n", THIS_MODULE->name);
            do_module_cleanup();
            break;
        }

        if (!device_create(string_ops_class, NULL, MKDEV(major_number, 0), NULL, "ioctlstringops"))
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

static void string_ops_exit(void)
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

        buffer_ptr = buffer;
        output_prefix_ptr = output_prefix;
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

static ssize_t device_read(struct file* filp, char* buffer, size_t length, loff_t* offset)
{
    ssize_t result = 0;

    const char* const result_buffer_start_ptr = buffer_ptr;

    if (!buffer_ptr)
    {
        result = -1;
    }
    else if (*buffer_ptr != '\0')
    {
        ssize_t bytes_read = 0;

        if (settings & OUTPUT_PREFIX_ENABLED)
        {
            char output_buf[2 * BUFFER_SIZE];
            char* output_buf_ptr = output_buf;
            memset(output_buf, '\0', 2 * BUFFER_SIZE);
            strncpy(output_buf, output_prefix, strlen(output_prefix));
            strncpy(output_buf + strlen(output_prefix), buffer_ptr, strlen(buffer_ptr));

            while (length && *output_buf_ptr)
            {
                put_user(*(output_buf_ptr++), buffer++);
                --length;
                bytes_read++;
            }

            result = bytes_read;
            pr_info("%s: user read: %s", THIS_MODULE->name, result_buffer_start_ptr);
        }
        else
        {

            while (length && *buffer_ptr)
            {
                put_user(*(buffer_ptr++), buffer++);
                --length;
                bytes_read++;
            }

            result = bytes_read;
            pr_info("%s: user read: %s", THIS_MODULE->name, result_buffer_start_ptr);
        }
    }

    return result;
}

static ssize_t device_write(struct file* filp, const char* buf, size_t length, loff_t* offset)
{
    ssize_t result = -EINVAL;

    char temp[BUFFER_SIZE];
    memset(temp, '\0', BUFFER_SIZE);

    const size_t max_bytes_to_copy_count = BUFFER_SIZE - 1;
    const size_t bytes_to_copy_count = length > max_bytes_to_copy_count ? max_bytes_to_copy_count : length;
    const size_t bytes_not_copied_count = copy_from_user(temp, buf, bytes_to_copy_count);

    if (bytes_not_copied_count > 0)
    {
        pr_warn("%s: %ld bytes could not be copied from user\n", THIS_MODULE->name, bytes_not_copied_count);
    }

    result =
        (ssize_t)strlen(temp); // the total number of chars provided by user (not the trimmed one) needs to be returned

    pr_info("%s: user wrote: %s\n", THIS_MODULE->name, temp);

    if (settings & TRIM_USER_INPUT_ENABLED)
    {
        trim_and_copy_string(buffer, temp, BUFFER_SIZE);
        pr_info("%s: after trimming the user provided string was stored as: \"%s\"\n", THIS_MODULE->name, buffer);
    }
    else
    {
        memset(buffer, '\0', BUFFER_SIZE);
        strncpy(buffer, temp, result);
        pr_info("%s: no trimming applied, the user provided string was stored as: \"%s\"\n", THIS_MODULE->name, buffer);
    }

    return result;
}

static long device_ioctl(struct file* file, unsigned int command, unsigned long arg)
{
    switch (command)
    {
    case GET_BUFFER_SIZE: {
        ioctl_get_buffer_size(buffer, (size_t*)arg);
        break;
    }
    case TRIM_USER_INPUT: {
        ioctl_trim_user_input(&settings, (uint8_t*)arg);
        break;
    }
    case DO_MODULE_RESET: {
        ioctl_do_module_reset(buffer, &settings);
        break;
    }
    case IS_MODULE_RESET: {
        ioctl_is_module_reset(buffer, &settings, (uint8_t*)arg);
        break;
    }
    case SET_OUTPUT_PREFIX: {
        ioctl_set_output_prefix(output_prefix, &settings, (void*)arg);
        break;
    }
    default:
        break;
    }

    return 0;
}

static void reset_buffers(void)
{
    memset(buffer, '\0', BUFFER_SIZE);
}

static void do_module_cleanup()
{
    if (string_ops_class)
    {
        device_destroy(string_ops_class, MKDEV(major_number, 0));
        class_destroy(string_ops_class);
    }

    cdev_del(&string_ops_cdev);
    unregister_chrdev(major_number, THIS_MODULE->name);
}

module_init(string_ops_init);
module_exit(string_ops_exit);
