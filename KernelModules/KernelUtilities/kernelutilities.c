#include <linux/ctype.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This module is a utitilies appliance used by other kernel modules.\n");
MODULE_AUTHOR("Liviu Popa");

void trim_and_copy_string(char* dest, const char* src, size_t max_str_length)
{
    do
    {
        if (max_str_length == 0)
        {
            pr_warn("%s: trim_and_copy_string: invalid maximum string length!\n", THIS_MODULE->name);
            break;
        }

        if (!dest)
        {
            pr_warn("%s: trim_and_copy_string: NULL dest string!\n", THIS_MODULE->name);
            break;
        }

        memset(dest, '\0', max_str_length);

        if (!src)
        {
            pr_warn("%s: trim_and_copy_string: NULL src string!\n", THIS_MODULE->name);
            break;
        }

        char* temp = kzalloc(max_str_length, GFP_KERNEL);

        if (temp == NULL)
        {
            pr_err("%s: trim_and_copy_string : memory could not be allocated!\n", THIS_MODULE->name);
            break;
        }

        memset(temp, '\0', max_str_length);
        strncpy(temp, src, max_str_length - 1);

        const size_t temp_length = strlen(temp);

        if (temp_length == 0)
        {
            break;
        }

        size_t left_index = 0;
        size_t right_index = temp_length - 1;

        while (left_index <= right_index && isspace(temp[left_index]))
        {
            ++left_index;
        }

        while (left_index < right_index && isspace(temp[right_index]))
        {
            --right_index;
        }

        const size_t length = right_index >= left_index ? right_index - left_index + 1 : 0;
        const char* start = temp + left_index;

        strncpy(dest, start, length);
        kfree(temp);
    } while (false);
}

int get_average(const int* array, size_t array_size)
{
    int sum = 0;

    for (size_t index = 0; index < array_size; ++index)
    {
        sum += array[index];
    }

    // it is assumed that array_size does not exceed the maximum int value and thus won't overflow when converting
    // size_t to int
    return array_size > 0 ? sum / (int)array_size : sum;
}

EXPORT_SYMBOL(trim_and_copy_string);
EXPORT_SYMBOL(get_average);

static int utilities_init(void)
{
    pr_info("%s: initializing module\n", THIS_MODULE->name);
    return 0;
}

static void utilities_exit(void)
{
    pr_info("%s: the module exited!\n", THIS_MODULE->name);
}

module_init(utilities_init);
module_exit(utilities_exit);
