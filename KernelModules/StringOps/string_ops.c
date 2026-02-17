#include <asm/uaccess.h>
#include <linux/ctype.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>

#define SUCCESS 0
#define INPUT_BUFFER_SIZE 128
#define OUTPUT_BUFFER_SIZE 256

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

static int major_number = 0;
static int minor_number;
static int is_device_open = 0;

static char input_buffer[INPUT_BUFFER_SIZE];          // shared input buffer (used by any minor number)
static char minor1_output_buffer[OUTPUT_BUFFER_SIZE]; // result buffer for minor 1
static char minor2_output_buffer[OUTPUT_BUFFER_SIZE]; // result buffer for minor 2
static char minor3_output_buffer[OUTPUT_BUFFER_SIZE]; // result buffer for minor 3

static char*
    result_buffer_ptr; // can point to any input/output buffer depending on minor number and operation (read/write)

extern void trim_and_copy_string(char* dest, const char* src, size_t max_str_length);

static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);
static ssize_t device_read(struct file*, char*, size_t, loff_t*);
static ssize_t device_write(struct file*, const char*, size_t, loff_t*);

static struct file_operations file_ops = {
    .read = device_read, .write = device_write, .open = device_open, .release = device_release};

static int string_ops_init(void)
{
    int result = SUCCESS;
    major_number = register_chrdev(major_number, THIS_MODULE->name, &file_ops);

    if (major_number >= 0)
    {
        pr_info("%s: device registered, major number %d successfully assigned\n", THIS_MODULE->name, major_number);
    }
    else
    {
        result = major_number;
        pr_alert("%s: registering char device failed with major number %d\n", THIS_MODULE->name, major_number);
    }

    return result;
}

static void string_ops_exit(void)
{
    unregister_chrdev(major_number, THIS_MODULE->name);
    pr_info("%s: device with major number %d unregistered\n", THIS_MODULE->name, major_number);
}

static int device_open(struct inode* inode, struct file* file)
{
    int result = SUCCESS;

    if (!is_device_open)
    {
        minor_number = iminor(inode);

        pr_info("%s: opening device, minor number is: %d\n", THIS_MODULE->name, minor_number);

        result_buffer_ptr = minor_number == 0   ? input_buffer
                            : minor_number == 1 ? minor1_output_buffer
                            : minor_number == 2 ? minor2_output_buffer
                            : minor_number == 3 ? minor3_output_buffer
                                                : NULL;
        if (result_buffer_ptr)
        {
            ++is_device_open;
            try_module_get(THIS_MODULE);
        }
        else
        {
            result = -1;
            pr_alert("%s: minor number should be between 0 and 3. %d is not supported!\n", THIS_MODULE->name,
                     minor_number);
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
    ssize_t result = 0;

    if (!result_buffer_ptr)
    {
        result = -1;
    }
    else if (*result_buffer_ptr != '\0')
    {
        ssize_t bytes_read = 0;

        const char* const result_buffer_start_ptr = result_buffer_ptr;

        while (length && *result_buffer_ptr)
        {
            put_user(*(result_buffer_ptr++), buffer++);
            --length;
            bytes_read++;
        }

        result = bytes_read;

        pr_info("%s: user read from minor number %d: %s", THIS_MODULE->name, minor_number, result_buffer_start_ptr);
    }

    return result;
}

static void convert_input_to_lower_case(void); // minor number 1 operation
static void reverse_input(void);               // minor number 2 operation
static void append_input_string_length(void);  // minor number 3 operation

static ssize_t device_write(struct file* filp, const char* buffer, size_t length, loff_t* offset)
{
    ssize_t result = -EINVAL;

    if (minor_number > 0)
    {
        memset(input_buffer, '\0', INPUT_BUFFER_SIZE);
        const size_t bytes_not_copied_count = copy_from_user(input_buffer, buffer, length);

        if (bytes_not_copied_count > 0)
        {
            pr_warn("%s: %ld bytes could not be copied from user\n", THIS_MODULE->name, bytes_not_copied_count);
        }

        result = (ssize_t)strlen(input_buffer);

        pr_info("%s: user wrote to minor number %d: %s", THIS_MODULE->name, minor_number, input_buffer);
    }

    switch (minor_number)
    {
    case 0: {
        pr_err("%s: unsupported write operation for minor number %d!\n", THIS_MODULE->name, minor_number);
        break;
    }
    case 1: {
        convert_input_to_lower_case();
        break;
    }
    case 2: {
        reverse_input();
        break;
    }
    case 3: {
        append_input_string_length();
        break;
    }
    default:
        break;
    }

    return result;
}

static void convert_input_to_lower_case(void)
{
    if (result_buffer_ptr)
    {
        memset(result_buffer_ptr, '\0', OUTPUT_BUFFER_SIZE);

        for (size_t index = 0; index < strlen(input_buffer); ++index)
        {
            result_buffer_ptr[index] = tolower(input_buffer[index]);
        }
    }
}

static void reverse_input(void)
{
    if (result_buffer_ptr)
    {
        memset(result_buffer_ptr, '\0', OUTPUT_BUFFER_SIZE);

        char temp[OUTPUT_BUFFER_SIZE];
        trim_and_copy_string(temp, input_buffer, INPUT_BUFFER_SIZE); // one byte for '\n', one byte for '\0'

        const size_t length = strlen(temp);

        for (size_t index = 0; index < length; ++index)
        {
            result_buffer_ptr[index] = temp[length - 1 - index];
        }

        result_buffer_ptr[length] = '\n';
    }
}

static void append_input_string_length(void)
{
    if (result_buffer_ptr)
    {
        char temp[OUTPUT_BUFFER_SIZE];
        trim_and_copy_string(temp, input_buffer, INPUT_BUFFER_SIZE);
        sprintf(result_buffer_ptr, "%s; %ld\n", temp, strlen(temp));
    }
}

module_init(string_ops_init);
module_exit(string_ops_exit);
