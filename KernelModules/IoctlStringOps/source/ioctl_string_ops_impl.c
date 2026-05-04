#include <linux/module.h>
#include <linux/uaccess.h>

#include "ioctl_string_ops_impl.h"

#define BUFFER_SIZE 1024
#define PREFIX_BUFFER_SIZE 128

#define TRIM_USER_INPUT_ENABLED 0b00000001
#define OUTPUT_PREFIX_ENABLED 0b00000010
#define USER_INPUT_APPENDING_ENABLED 0b00100000
#define DEFAULT_SETTINGS 0b00000001

static char buffer[BUFFER_SIZE]; // shared input/output buffer
static char output_prefix[PREFIX_BUFFER_SIZE];

/* 0: trim user input
   1: enable output prefix
   2: input mode
   3-7: reserved for future use
*/
static uint8_t settings = DEFAULT_SETTINGS;

extern void trim_and_copy_string(char* dest, const char* src, size_t max_str_length, const char* calling_module_name);

ssize_t device_read_impl(struct file* filp, char* buf, size_t length, loff_t* offset)
{
    ssize_t bytes_read = 0;
    char* output_buffer_ptr = buffer;

    if (settings & OUTPUT_PREFIX_ENABLED)
    {
        char consolidated_output_buffer[PREFIX_BUFFER_SIZE + BUFFER_SIZE];

        output_buffer_ptr = consolidated_output_buffer;
        memset(output_buffer_ptr, '\0', PREFIX_BUFFER_SIZE + BUFFER_SIZE);

        const size_t output_prefix_size = strlen(output_prefix);
        const size_t buffer_size = strlen(buffer);

        strncpy(output_buffer_ptr, output_prefix, output_prefix_size);
        strncpy(output_buffer_ptr + output_prefix_size, buffer, buffer_size);
    }

    while (length && *output_buffer_ptr)
    {
        put_user(*(output_buffer_ptr++), buf++);
        --length;
        bytes_read++;
    }

    if (bytes_read > 0)
    {
        pr_info("%s: user read: %s", THIS_MODULE->name, buffer);
    }

    return bytes_read;
}

ssize_t device_write_impl(struct file* filp, const char* buf, size_t length, loff_t* offset)
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

    char input_buffer[BUFFER_SIZE];

    if (settings & TRIM_USER_INPUT_ENABLED)
    {
        trim_and_copy_string(input_buffer, temp, BUFFER_SIZE, THIS_MODULE->name);
        pr_info("%s: after trimming the user provided string was stored as: \"%s\"\n", THIS_MODULE->name, input_buffer);
    }
    else
    {
        memset(input_buffer, '\0', BUFFER_SIZE);
        strncpy(input_buffer, temp, result);
        pr_info("%s: no trimming applied, the user provided string was stored as: \"%s\"\n", THIS_MODULE->name,
                input_buffer);
    }

    if (settings & USER_INPUT_APPENDING_ENABLED)
    {
        const size_t max_chars_count = BUFFER_SIZE - 1;
        const size_t buffer_chars_count = strlen(buffer);
        const size_t available_chars_count = max_chars_count - buffer_chars_count;
        const size_t chars_to_append_count = (size_t)result;

        if (available_chars_count >= chars_to_append_count)
        {
            strncpy(buffer + buffer_chars_count, input_buffer, chars_to_append_count);
            pr_info("%s: the input has been appended to the driver buffer\n", THIS_MODULE->name);
        }
        else
        {
            pr_warn("%s: the input could not be appended to the driver buffer. There is not enough space.\n",
                    THIS_MODULE->name);
        }
    }
    else
    {
        memset(buffer, '\0', BUFFER_SIZE);
        strncpy(buffer, input_buffer, result);
    }

    return result;
}

long ioctl_do_module_reset()
{
    reset_buffers();
    settings = DEFAULT_SETTINGS;

    return 0;
}

long ioctl_is_module_reset(uint8_t* is_module_reset)
{
    long result = -1;

    if (is_module_reset)
    {
        const uint8_t is_reset = (settings == DEFAULT_SETTINGS && strlen(buffer) == 0);
        const size_t bytes_not_copied_count = copy_to_user(is_module_reset, &is_reset, sizeof(is_reset));

        if (bytes_not_copied_count == 0)
        {
            result = 0;
        }
        else
        {
            pr_err("%s: IOCTL: failed checking if the module is reset!\n", THIS_MODULE->name);
        }
    }

    return result;
}

long ioctl_trim_user_input(const uint8_t* should_trim)
{
    long result = -1;

    do
    {
        if (!should_trim)
        {
            break;
        }

        uint8_t should_trim_user_input;
        const size_t bytes_not_copied_count = copy_from_user(&should_trim_user_input, should_trim, sizeof(uint8_t));

        if (bytes_not_copied_count > 0)
        {
            pr_err("%s: IOCTL: failed updating the \"should trim user input\" setting!\n", THIS_MODULE->name);
            break;
        }

        if (should_trim_user_input)
        {
            settings |= TRIM_USER_INPUT_ENABLED;
        }
        else
        {
            settings &= ~TRIM_USER_INPUT_ENABLED;
        }

        result = 0;
    } while (false);

    return result;
}

long ioctl_get_chars_count_from_buffer(size_t* buffer_size)
{
    long result = -1;

    if (buffer_size)
    {
        const size_t buffer_length = strlen(buffer);
        const size_t bytes_not_copied_count = copy_to_user(buffer_size, &buffer_length, sizeof(buffer_length));

        if (bytes_not_copied_count == 0)
        {
            result = 0;
        }
        else
        {
            pr_err("%s: IOCTL: failed reading chars count from buffer!\n", THIS_MODULE->name);
        }
    }

    return result;
}

long ioctl_set_output_prefix(const void* output_prefix_data)
{
    uint8_t success = 0;

    do
    {
        if (!output_prefix_data)
        {
            break;
        }

        size_t prefix_size;
        size_t bytes_not_copied_count = copy_from_user(&prefix_size, output_prefix_data, sizeof(prefix_size));

        if (bytes_not_copied_count > 0 || prefix_size >= PREFIX_BUFFER_SIZE)
        {
            break;
        }

        if (prefix_size == 0)
        {
            memset(output_prefix, '\0', PREFIX_BUFFER_SIZE);
            settings &= ~OUTPUT_PREFIX_ENABLED;
            success = 1;
            break;
        }

        output_prefix_data = (char*)output_prefix_data + sizeof(prefix_size);

        char temp[PREFIX_BUFFER_SIZE];

        memset(temp, '\0', PREFIX_BUFFER_SIZE);
        bytes_not_copied_count = copy_from_user(temp, output_prefix_data, prefix_size);

        if (bytes_not_copied_count > 0 || strlen(temp) != prefix_size)
        {
            break;
        }

        memset(output_prefix, '\0', PREFIX_BUFFER_SIZE);
        strncpy(output_prefix, temp, prefix_size);
        settings |= OUTPUT_PREFIX_ENABLED;
        success = 1;
    } while (false);

    if (!success)
    {
        pr_err("%s: IOCTL: failed setting the output prefix!\n", THIS_MODULE->name);
    }

    return success ? 0 : -1;
}

long ioctl_get_output_prefix_size(size_t* output_prefix_size)
{
    long result = -1;

    if (output_prefix_size)
    {
        const size_t module_output_prefix_length = strlen(output_prefix);
        const size_t bytes_not_copied_count =
            copy_to_user(output_prefix_size, &module_output_prefix_length, sizeof(module_output_prefix_length));

        if (bytes_not_copied_count == 0)
        {
            result = 0;
        }
        else
        {
            pr_err("%s: IOCTL: failed reading output prefix size!\n", THIS_MODULE->name);
        }
    }

    return result;
}

long ioctl_enable_input_append_mode(uint8_t* should_append)
{
    long result = -1;

    do
    {
        if (!should_append)
        {
            break;
        }

        uint8_t should_append_user_input;
        const size_t bytes_not_copied_count = copy_from_user(&should_append_user_input, should_append, sizeof(uint8_t));

        if (bytes_not_copied_count > 0)
        {
            pr_err("%s: IOCTL: failed updating the \"input mode\" setting!\n", THIS_MODULE->name);
            break;
        }

        if (should_append_user_input)
        {
            settings |= USER_INPUT_APPENDING_ENABLED;
        }
        else
        {
            settings &= ~USER_INPUT_APPENDING_ENABLED;
        }

        result = 0;
    } while (false);

    return result;
}

long ioctl_is_input_append_mode_enabled(uint8_t* is_append_enabled)
{
    long result = -1;

    if (is_append_enabled)
    {
        const uint8_t is_enabled = settings & USER_INPUT_APPENDING_ENABLED;
        const size_t bytes_not_copied_count = copy_to_user(is_append_enabled, &is_enabled, sizeof(is_enabled));

        if (bytes_not_copied_count == 0)
        {
            result = 0;
        }
        else
        {
            pr_err("%s: IOCTL: failed checking if the user input append mode is enabled!\n", THIS_MODULE->name);
        }
    }

    return result;
}

void reset_buffers(void)
{
    memset(buffer, '\0', BUFFER_SIZE);
    memset(output_prefix, '\0', PREFIX_BUFFER_SIZE);
}
