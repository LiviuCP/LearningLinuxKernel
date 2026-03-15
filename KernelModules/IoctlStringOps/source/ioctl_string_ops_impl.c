#include <linux/module.h>
#include <linux/uaccess.h>

#include "ioctl_string_ops_impl.h"

void ioctl_get_buffer_size(char* module_buffer, size_t* buffer_size)
{
    if (module_buffer && buffer_size)
    {
        const size_t module_buffer_length = strlen(module_buffer);
        const int result = copy_to_user(buffer_size, &module_buffer_length, sizeof(module_buffer_length));

        if (result)
        {
            pr_err("%s: IOCTL - failed reading buffer size!\n", THIS_MODULE->name);
        }
    }
}

void ioctl_trim_user_input(uint8_t* settings, uint8_t* should_trim)
{
    if (settings && should_trim)
    {
        uint8_t should_trim_user_input;
        const int result = copy_from_user(&should_trim_user_input, (uint8_t*)should_trim, sizeof(uint8_t));
        if (result)
        {
            pr_err("%s: IOCTL - failed reading the should trim user input variable\n", THIS_MODULE->name);
        }
        else
        {
            if (should_trim_user_input)
            {
                *settings |= TRIM_USER_INPUT_ENABLED;
            }
            else
            {
                *settings &= ~TRIM_USER_INPUT_ENABLED;
            }
        }
    }
}

void ioctl_do_module_reset(char* module_buffer, uint8_t* settings)
{
    if (module_buffer && settings)
    {
        memset(module_buffer, '\0', BUFFER_SIZE);
        *settings = DEFAULT_SETTINGS;
    }
}

void ioctl_is_module_reset(char* module_buffer, uint8_t* settings, uint8_t* is_module_reset)
{
    if (module_buffer && settings && is_module_reset)
    {
        const uint8_t is_reset = (*settings == DEFAULT_SETTINGS && strlen(module_buffer) == 0);
        const int result = copy_to_user(is_module_reset, &is_reset, sizeof(is_reset));
        if (result)
        {
            pr_err("%s: IOCTL - failed checking if it is reset!\n", THIS_MODULE->name);
        }
    }
}

void ioctl_set_output_prefix(char* output_prefix, uint8_t* settings, void* output_prefix_data)
{
    for (;;)
    {
        if (!output_prefix || !settings || !output_prefix_data)
        {
            break;
        }
        size_t prefix_size;
        int result = copy_from_user(&prefix_size, output_prefix_data, sizeof(prefix_size));
        if (result || prefix_size >= BUFFER_SIZE)
        {
            break;
        }
        if (prefix_size == 0)
        {
            *settings &= ~OUTPUT_PREFIX_ENABLED;
            break;
        }
        char prefix_buffer[BUFFER_SIZE];
        memset(prefix_buffer, '\0', BUFFER_SIZE);
        output_prefix_data = (char*)output_prefix_data + sizeof(prefix_size);
        result = copy_from_user(prefix_buffer, output_prefix_data, prefix_size);
        if (result || strlen(prefix_buffer) != prefix_size)
        {
            pr_err("%s: IOCTL - unable to set prefix\n", THIS_MODULE->name);
        }
        memset(output_prefix, '\0', BUFFER_SIZE);
        strncpy(output_prefix, prefix_buffer, prefix_size);
        *settings |= OUTPUT_PREFIX_ENABLED;
    }
}
