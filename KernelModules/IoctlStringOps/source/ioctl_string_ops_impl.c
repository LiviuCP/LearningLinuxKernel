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
