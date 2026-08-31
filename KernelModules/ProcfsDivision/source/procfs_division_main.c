#include <linux/module.h>
#include <linux/proc_fs.h>

#include "procfs_division_impl.h"

#define SUCCESS 0

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This module divides two integers and provides the quotient and remainder. Six attributes are "
                   "being used: 2 are read/write (divided/divider), 3 are read-only (quotient/remainder/status), 1 is "
                   "write-only (command).\n");
MODULE_AUTHOR("Liviu Popa");

/* VARIABLES */

static int divided = 0;
static int divider = 1;

static struct division_data* data = NULL;
static struct proc_dir_entry* division_dir = NULL;

// temporary buffer for storing data read from user buffer or data to be written to user buffer
static char temp_buffer[MAX_CHARS_COUNT + 1];

/* HELPER functions */

static int copy_from_user_buffer(const char* user_buffer, size_t length)
{
    memset(temp_buffer, '\0', sizeof(temp_buffer));

    const size_t charsToCopyCount = length < MAX_CHARS_COUNT ? length : MAX_CHARS_COUNT;
    const int result = copy_from_user(temp_buffer, user_buffer, charsToCopyCount);

    return result;
}

/* PROCFS access functions for files */

static ssize_t divided_show(struct file* filp, char* buffer, size_t length, loff_t* offset)
{
    memset(temp_buffer, '\0', sizeof(temp_buffer));
    sprintf(temp_buffer, "%d\n", data->divided);

    const int result = copy_to_user(buffer, temp_buffer, strlen(temp_buffer));

    if (result != SUCCESS)
    {
        pr_err("%s: error reading divided value!\n", THIS_MODULE->name);
    }

    return length;
}

static ssize_t divided_store(struct file* filp, const char* buffer, size_t length, loff_t* offset)
{
    int result = -1;
    const int copy_result = copy_from_user_buffer(buffer, length);

    if (copy_result == SUCCESS)
    {
        result = store_divided_value(data, temp_buffer);
    }
    else
    {
        pr_err("%s: error writing divided value!\n", THIS_MODULE->name);
    }

    return result < 0 ? 0 : length;
}

static ssize_t divider_show(struct file* filp, char* buffer, size_t length, loff_t* offset)
{
    memset(temp_buffer, '\0', sizeof(temp_buffer));
    sprintf(temp_buffer, "%d\n", data->divider);

    const int result = copy_to_user(buffer, temp_buffer, strlen(temp_buffer));

    if (result != SUCCESS)
    {
        pr_err("%s: error reading divider value!\n", THIS_MODULE->name);
    }

    return length;
}

static ssize_t divider_store(struct file* filp, const char* buffer, size_t length, loff_t* offset)
{
    int result = -1;
    const int copy_result = copy_from_user_buffer(buffer, length);

    if (copy_result == SUCCESS)
    {
        result = store_divider_value(data, temp_buffer);
    }
    else
    {
        pr_err("%s: error writing divided value!\n", THIS_MODULE->name);
    }

    return result < 0 ? 0 : length;
}

// no store to be defined here as the quotient is read-only
static ssize_t quotient_show(struct file* filp, char* buffer, size_t length, loff_t* offset)
{
    memset(temp_buffer, '\0', sizeof(temp_buffer));
    sprintf(temp_buffer, "%d\n", data->quotient);

    const int result = copy_to_user(buffer, temp_buffer, strlen(temp_buffer));

    if (result != SUCCESS)
    {
        pr_err("%s: error reading quotient value!\n", THIS_MODULE->name);
    }

    return length;
}

// same here
static ssize_t remainder_show(struct file* filp, char* buffer, size_t length, loff_t* offset)
{
    memset(temp_buffer, '\0', sizeof(temp_buffer));
    sprintf(temp_buffer, "%d\n", data->remainder);

    const int result = copy_to_user(buffer, temp_buffer, strlen(temp_buffer));

    if (result != SUCCESS)
    {
        pr_err("%s: error reading remainder value!\n", THIS_MODULE->name);
    }

    return length;
}

// no show to be defined here as the command is write-only
static ssize_t command_store(struct file* filp, const char* buffer, size_t length, loff_t* offset)
{
    const int result = copy_from_user_buffer(buffer, length);

    if (result == SUCCESS)
    {
        store_command(data, temp_buffer);
    }
    else
    {
        pr_err("%s: error writing command value!\n", THIS_MODULE->name);
    }

    return length;
}

// no store to be defined here as the status is read-only
static ssize_t status_show(struct file* filp, char* buffer, size_t length, loff_t* offset)
{
    memset(temp_buffer, '\0', sizeof(temp_buffer));
    sprintf(temp_buffer, "%s\n", data->status);

    const int result = copy_to_user(buffer, temp_buffer, strlen(temp_buffer));

    if (result != SUCCESS)
    {
        pr_err("%s: error reading status value!\n", THIS_MODULE->name);
    }

    return length;
}

static int open_file(struct inode* inode, struct file* file)
{
    return SUCCESS;
}

static int release_file(struct inode* inode, struct file* file)
{
    return SUCCESS;
}

/* REQUIRED STRUCTURES for PROCFS files */

static struct proc_ops divided_fops = {
    .proc_open = open_file, .proc_read = divided_show, .proc_write = divided_store, .proc_release = release_file};

static struct proc_ops divider_fops = {
    .proc_open = open_file, .proc_read = divider_show, .proc_write = divider_store, .proc_release = release_file};

static struct proc_ops quotient_fops = {
    .proc_open = open_file, .proc_read = quotient_show, .proc_write = NULL, .proc_release = release_file};

static struct proc_ops remainder_fops = {
    .proc_open = open_file, .proc_read = remainder_show, .proc_write = NULL, .proc_release = release_file};

static struct proc_ops command_fops = {
    .proc_open = open_file, .proc_read = NULL, .proc_write = command_store, .proc_release = release_file};

static struct proc_ops status_fops = {
    .proc_open = open_file, .proc_read = status_show, .proc_write = NULL, .proc_release = release_file};

/* INIT/EXIT */

static int division_init(void)
{
    int result = -1;

    pr_info("%s: initializing module\n", THIS_MODULE->name);

    division_dir = proc_mkdir("division", NULL);

    if (division_dir)
    {
        data = kzalloc(sizeof(struct division_data), GFP_KERNEL);
        result = data ? init_data(data, divided, divider) : -ENOMEM;
    }

    if (result == SUCCESS)
    {
        proc_create("divided", 0666, division_dir, &divided_fops);
        proc_create("divider", 0666, division_dir, &divider_fops);
        proc_create("quotient", 0444, division_dir, &quotient_fops);
        proc_create("remainder", 0444, division_dir, &remainder_fops);
        proc_create("command", 0222, division_dir, &command_fops);
        proc_create("status", 0444, division_dir, &status_fops);

        pr_info(
            "%s: Successfully created division directory and its files in /proc and initialized division operations!\n",
            THIS_MODULE->name);
    }
    else
    {
        pr_err("%s: unable to initialize division operations!\n", THIS_MODULE->name);
    }

    return result;
}

static void division_exit(void)
{
    kfree(data);
    proc_remove(division_dir);
    pr_info("%s: the module exited!\n", THIS_MODULE->name);
}

module_init(division_init);
module_exit(division_exit);
