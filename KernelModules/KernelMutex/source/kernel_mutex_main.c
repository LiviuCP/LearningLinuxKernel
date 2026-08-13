#include <linux/cdev.h>
#include <linux/fs.h>

#include "kernel_mutex_impl.h"

#define SUCCESS 0
#define SUPPORTED_MINOR_NUMBERS_COUNT 1

MODULE_LICENSE("GPL");

MODULE_DESCRIPTION("This driver module illustrates kernel threads and kernel mutex.\n"
                   "Two threads attempt to repeatedly print \"S.O.S\" and \"S.M.S\".\n"
                   "They print each of these characters on a separate line by using the kernel ring buffer.\n"
                   "The driver read operation toggles the S.O.S. thread on/off.\n"
                   "The driver write operation toggles the S.M.S. thread on/off.\n"
                   "Any running thread is stopped by the exit function.\n"
                   "The ring buffer needs to be protected by a mutex to ensure in-order message display.\n");

MODULE_AUTHOR("Liviu Popa");

static struct class* kernel_mutex_class = NULL;
static struct cdev kernel_mutex_cdev;

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

static int kernel_mutex_init(void)
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

        cdev_init(&kernel_mutex_cdev, &file_ops);

        int cdev_add_result = cdev_add(&kernel_mutex_cdev, major_number, SUPPORTED_MINOR_NUMBERS_COUNT);

        if (cdev_add_result < 0)
        {
            pr_alert("%s: cannot add device to the system\n", THIS_MODULE->name);
            do_module_cleanup();
            break;
        }

        kernel_mutex_class = class_create("kernel_mutex_class");

        if (!kernel_mutex_class)
        {
            pr_alert("%s: cannot create the struct class (kernel_mutex_class)\n", THIS_MODULE->name);
            do_module_cleanup();
            break;
        }

        if (!device_create(kernel_mutex_class, NULL, MKDEV(major_number, 0), NULL, "kernelmutex"))
        {
            pr_alert("%s: cannot create the device!\n", THIS_MODULE->name);
            do_module_cleanup();
            break;
        }

        init_threads();

        result = SUCCESS;
        pr_info("%s: device registered, major number %d successfully assigned\n", THIS_MODULE->name, major_number);
    } while (false);

    return result;
}

static void kernel_mutex_exit(void)
{
    clear_threads();
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

static void do_module_cleanup()
{
    if (kernel_mutex_class)
    {
        device_destroy(kernel_mutex_class, MKDEV(major_number, 0));
        class_destroy(kernel_mutex_class);
    }

    cdev_del(&kernel_mutex_cdev);
    unregister_chrdev(major_number, THIS_MODULE->name);
}

module_init(kernel_mutex_init);
module_exit(kernel_mutex_exit);
