#include <linux/module.h>
#include <linux/uaccess.h>

#include "ioctl_string_ops_impl.h"

long ioctl_trim_user_input(const uint8_t* should_trim, uint8_t* module_settings)
{
    long result = -1;

    for (;;)
    {
        if (!should_trim || !module_settings)
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
            *module_settings |= TRIM_USER_INPUT_ENABLED;
        }
        else
        {
            *module_settings &= ~TRIM_USER_INPUT_ENABLED;
        }

        result = 0;
        break;
    }

    return result;
}

long ioctl_get_chars_count_from_buffer(const char* module_buffer, size_t* buffer_size)
{
    long result = -1;

    if (module_buffer && buffer_size)
    {
        const size_t module_buffer_length = strlen(module_buffer);
        const size_t bytes_not_copied_count =
            copy_to_user(buffer_size, &module_buffer_length, sizeof(module_buffer_length));

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

long ioctl_set_output_prefix(const void* output_prefix_data, char* module_output_prefix, uint8_t* module_settings)
{
    uint8_t success = 0;

    for (;;)
    {
        if (!output_prefix_data || !module_output_prefix || !module_settings)
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
            memset(module_output_prefix, '\0', PREFIX_BUFFER_SIZE);
            *module_settings &= ~OUTPUT_PREFIX_ENABLED;
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

        memset(module_output_prefix, '\0', PREFIX_BUFFER_SIZE);
        strncpy(module_output_prefix, temp, prefix_size);
        *module_settings |= OUTPUT_PREFIX_ENABLED;
        success = 1;
        break;
    }

    if (!success)
    {
        pr_err("%s: IOCTL: failed setting the output prefix!\n", THIS_MODULE->name);
    }

    return success ? 0 : -1;
}

long ioctl_get_output_prefix_size(const char* module_output_prefix, size_t* output_prefix_size)
{
    long result = -1;

    if (module_output_prefix && output_prefix_size)
    {
        const size_t module_output_prefix_length = strlen(module_output_prefix);
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

long ioctl_do_module_reset(char* module_buffer, char* module_output_prefix, uint8_t* module_settings)
{
    long result = -1;

    if (module_buffer && module_settings)
    {
        reset_buffers(module_buffer, module_output_prefix);
        *module_settings = DEFAULT_SETTINGS;
        result = 0;
    }

    return result;
}

long ioctl_is_module_reset(const char* module_buffer, const uint8_t* module_settings, uint8_t* is_module_reset)
{
    long result = -1;

    if (module_buffer && module_settings && is_module_reset)
    {
        const uint8_t is_reset = (*module_settings == DEFAULT_SETTINGS && strlen(module_buffer) == 0);
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

void reset_buffers(char* module_buffer, char* module_output_prefix)
{
    if (module_buffer)
    {
        memset(module_buffer, '\0', BUFFER_SIZE);
    }

    if (module_output_prefix)
    {
        memset(module_output_prefix, '\0', PREFIX_BUFFER_SIZE);
    }
}
