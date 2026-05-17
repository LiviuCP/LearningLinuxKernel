#include <linux/module.h>

#include "string_ops_impl.h"

extern void trim_and_copy_string(char* dest, const char* src, size_t max_str_length, const char* calling_module_name);
extern void convert_to_same_case_and_copy_string(char* dest, const char* src, size_t chars_count, int to_lower_case,
                                                 const char* calling_module_name); // minor 1 operation
extern void reverse_and_copy_string(char* dest, const char* src, size_t chars_count,
                                    const char* calling_module_name); // minor 2 operation

static char input_buffer[INPUT_BUFFER_SIZE]; // shared input buffer (used by any minor number)
static char minor1_buffer[DATA_BUFFER_SIZE]; // data buffer for minor 1
static char minor2_buffer[DATA_BUFFER_SIZE]; // data buffer for minor 2
static char minor3_buffer[DATA_BUFFER_SIZE]; // data buffer for minor 3

static char* current_buffer_ptr = NULL; // can point to any buffer depending on minor number and operation (read/write)

static void append_input_string_length(void)
{
    if (current_buffer_ptr)
    {
        const size_t length = strlen(input_buffer);

        if (length > 0)
        {
            sprintf(current_buffer_ptr, "%s; %ld", input_buffer, length);
        }
        else
        {
            memset(current_buffer_ptr, '\0', DATA_BUFFER_SIZE);
        }
    }
}

static void write_to_current_buffer(int minor_number, const char* raw_input_buffer)
{
    trim_and_copy_string(input_buffer, raw_input_buffer, INPUT_BUFFER_SIZE, THIS_MODULE->name);

    pr_info("%s: after trimming the user provided string was stored to minor number %d as: %s\n", THIS_MODULE->name,
            minor_number, input_buffer);

    switch (minor_number)
    {
    case 1: {
        convert_to_same_case_and_copy_string(current_buffer_ptr, input_buffer, DATA_BUFFER_SIZE, TO_LOWER_CASE,
                                             THIS_MODULE->name);
        break;
    }
    case 2: {
        reverse_and_copy_string(current_buffer_ptr, input_buffer, DATA_BUFFER_SIZE, THIS_MODULE->name);
        break;
    }
    case 3: {
        append_input_string_length();
        break;
    }
    default:
        break;
    }
}

static bool can_write_to_minor_number(int minor_number)
{
    return is_valid_minor_number(minor_number) && minor_number != 0;
}

ssize_t device_read_impl(struct file* filp, char* buffer, size_t length, loff_t* offset, int minor_number)
{
    ssize_t result = -1;

    if (current_buffer_ptr)
    {
        ssize_t read_bytes_count = 0;
        const char* const output_buffer_start_ptr = current_buffer_ptr;

        while (length && *current_buffer_ptr != '\0')
        {
            put_user(*(current_buffer_ptr++), buffer++);
            --length;
            read_bytes_count++;
        }

        result = read_bytes_count;

        pr_info("%s: user read from minor number %d: %s", THIS_MODULE->name, minor_number, output_buffer_start_ptr);
    }
    else
    {
        // defensive programming, output buffer should be set in read mode
        pr_err("%s: cannot read, NULL output buffer!", THIS_MODULE->name);
    }

    return result;
}

ssize_t device_write_impl(struct file* filp, const char* buffer, size_t length, loff_t* offset, int minor_number)
{
    ssize_t result = -EINVAL;

    if (can_write_to_minor_number(minor_number))
    {
        char raw_input_buffer[INPUT_BUFFER_SIZE];
        memset(raw_input_buffer, '\0', INPUT_BUFFER_SIZE);

        const size_t max_bytes_to_copy_count = INPUT_BUFFER_SIZE - 1;
        const size_t bytes_to_copy_count = length > max_bytes_to_copy_count ? max_bytes_to_copy_count : length;
        const size_t bytes_not_copied_count = copy_from_user(raw_input_buffer, buffer, bytes_to_copy_count);

        if (bytes_not_copied_count > 0)
        {
            pr_warn("%s: %ld bytes could not be copied from user\n", THIS_MODULE->name, bytes_not_copied_count);
        }

        result = (ssize_t)strlen(
            raw_input_buffer); // the total number of chars provided by user (not the trimmed one) needs to be returned

        pr_info("%s: user wrote to minor number %d: %s\n", THIS_MODULE->name, minor_number, raw_input_buffer);

        write_to_current_buffer(minor_number, raw_input_buffer);
    }
    else
    {
        pr_err("%s: unsupported write operation for minor number %d!\n", THIS_MODULE->name, minor_number);
    }

    return result;
}

void link_minor_number_data(int minor_number)
{
    current_buffer_ptr = minor_number == 0   ? input_buffer
                         : minor_number == 1 ? minor1_buffer
                         : minor_number == 2 ? minor2_buffer
                         : minor_number == 3 ? minor3_buffer
                                             : NULL;
}

bool is_valid_minor_number(int minor_number)
{
    return minor_number >= 0 && minor_number < SUPPORTED_MINOR_NUMBERS_COUNT;
}

void reset_module_data(void)
{
    memset(input_buffer, '\0', INPUT_BUFFER_SIZE);
    memset(minor1_buffer, '\0', DATA_BUFFER_SIZE);
    memset(minor2_buffer, '\0', DATA_BUFFER_SIZE);
    memset(minor3_buffer, '\0', DATA_BUFFER_SIZE);
}
