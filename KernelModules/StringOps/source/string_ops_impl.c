#include <linux/module.h>

#include "string_ops_impl.h"

#define MAX_STR_TO_APPEND_CHARS_COUNT 16

extern bool trim_and_copy_string(char* dest, const char* src, size_t max_str_length, const char* calling_module_name);
extern bool convert_to_same_case_and_copy_string(char* dest, const char* src, size_t chars_count, int to_lower_case,
                                                 const char* calling_module_name); // minor 1 operation
extern bool reverse_and_copy_string(char* dest, const char* src, size_t chars_count,
                                    const char* calling_module_name); // minor 2 operation

static char input_buffer[INPUT_BUFFER_SIZE]; // shared input buffer (used by any minor number)
static char minor1_buffer[DATA_BUFFER_SIZE]; // data buffer for minor 1
static char minor2_buffer[DATA_BUFFER_SIZE]; // data buffer for minor 2
static char minor3_buffer[DATA_BUFFER_SIZE]; // data buffer for minor 3

static char* current_buffer_ptr = NULL; // can point to any buffer depending on minor number and operation (read/write)

static bool append_input_string_length(void)
{
    bool success = false;

    if (current_buffer_ptr)
    {
        const size_t length = strlen(input_buffer);

        if (length > 0)
        {
            char str_to_append[MAX_STR_TO_APPEND_CHARS_COUNT];

            memset(str_to_append, '\0', MAX_STR_TO_APPEND_CHARS_COUNT);
            sprintf(str_to_append, "; %ld", length);

            const size_t current_buffer_length = strlen(current_buffer_ptr);

            // terminating '\0' taken into account
            const size_t max_available_positions_count =
                current_buffer_length < DATA_BUFFER_SIZE ? DATA_BUFFER_SIZE - current_buffer_length - 1 : 0;

            if (strlen(str_to_append) <= max_available_positions_count)
            {
                sprintf(current_buffer_ptr, "%s%s", input_buffer, str_to_append);
                success = true;
            }
        }
        else
        {
            memset(current_buffer_ptr, '\0', DATA_BUFFER_SIZE);
            success = true;
        }
    }

    return success;
}

static bool write_to_current_data_buffer(int minor_number)
{
    bool success = false;

    switch (minor_number)
    {
    case 1: {
        success = convert_to_same_case_and_copy_string(current_buffer_ptr, input_buffer, DATA_BUFFER_SIZE,
                                                       TO_LOWER_CASE, THIS_MODULE->name);
        break;
    }
    case 2: {
        success = reverse_and_copy_string(current_buffer_ptr, input_buffer, DATA_BUFFER_SIZE, THIS_MODULE->name);
        break;
    }
    case 3: {
        success = append_input_string_length();
        break;
    }
    default:
        break;
    }

    return success;
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

    do
    {
        if (!can_write_to_minor_number(minor_number))
        {
            pr_err("%s: unsupported write operation for minor number %d!\n", THIS_MODULE->name, minor_number);
            break;
        }

        const size_t max_bytes_to_copy_count = INPUT_BUFFER_SIZE - 1;

        if (length > max_bytes_to_copy_count)
        {
            pr_err("%s: not enough space in input buffer to copy from raw input buffer!", THIS_MODULE->name);
            break;
        }

        char raw_input_buffer[INPUT_BUFFER_SIZE];
        memset(raw_input_buffer, '\0', INPUT_BUFFER_SIZE);

        const size_t bytes_not_copied_count = copy_from_user(raw_input_buffer, buffer, length);

        if (bytes_not_copied_count > 0)
        {
            pr_err("%s: %ld bytes could not be copied from user\n", THIS_MODULE->name, bytes_not_copied_count);
            break;
        }

        pr_info("%s: user wrote to minor number %d: %s\n", THIS_MODULE->name, minor_number, raw_input_buffer);

        if (strlen(raw_input_buffer) == 0)
        {
            memset(input_buffer, '\0', INPUT_BUFFER_SIZE);
        }
        else
        {
            if (!trim_and_copy_string(input_buffer, raw_input_buffer, INPUT_BUFFER_SIZE, THIS_MODULE->name))
            {
                pr_err("%s: cannot trim and copy to input buffer!\n", THIS_MODULE->name);
                break;
            }
        }

        pr_info("%s: after trimming the user provided string was stored to input buffer as: %s\n", THIS_MODULE->name,
                input_buffer);

        if (!write_to_current_data_buffer(minor_number))
        {
            pr_err("%s: an error occurred when updating the data buffer for minor number %d! The data buffer remains "
                   "unchanged.\n",
                   THIS_MODULE->name, minor_number);
            break;
        }

        // the total number of chars provided from user space needs to be returned
        // otherwise the function might get called again (possibly indefinitely) when buffer size is exceeded
        result = (ssize_t)length;
    } while (false);

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
