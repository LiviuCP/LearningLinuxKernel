#include <linux/ctype.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This module is a utitilies appliance used by other kernel modules.\n");
MODULE_AUTHOR("Liviu Popa");

static bool can_copy_to_destination(const char* dest, const char* src, size_t max_chars_count,
                                    const char* calling_module_name, const char* calling_function_name)
{
    bool can_copy = false;

    const char* module_name = calling_module_name ? calling_module_name : "INVALID MODULE NAME";
    const char* function_name = calling_function_name ? calling_function_name : "INVALID FUNCTION NAME";

    do
    {
        if (!dest)
        {
            pr_warn("%s: %s: NULL dest string!\n", module_name, function_name);
            break;
        }

        if (!src)
        {
            pr_warn("%s: %s: NULL src string!\n", module_name, function_name);
            break;
        }

        if (max_chars_count == 0)
        {
            pr_warn("%s: %s: invalid maximum dest string length!\n", module_name, function_name);
            break;
        }

        const size_t src_length = strlen(src);
        const size_t max_dest_length =
            max_chars_count - 1; // (at least one) terminating '\0' char included in max_chars_count

        if (src_length > max_dest_length)
        {
            pr_warn("%s: %s: src length exceeds maximum dest string length!\n", module_name, function_name);
            break;
        }

        const char* last_dest_char_ptr = dest + max_dest_length; // (at least one) terminating '\0' char included
        const char* last_src_char_ptr = src + src_length;        // terminating '\0' char included

        const bool src_and_dest_overlap = dest <= last_src_char_ptr && src <= last_dest_char_ptr;

        if (src_and_dest_overlap)
        {
            pr_warn("%s: %s: cannot copy source to destination, the strings overlap!\n", module_name, function_name);
            break;
        }

        can_copy = true;
    } while (false);

    return can_copy;
}

void trim_and_copy_string(char* dest, const char* src, size_t max_chars_count, const char* calling_module_name)
{
    const char* module_name = calling_module_name ? calling_module_name : "INVALID MODULE NAME";

    do
    {
        if (!can_copy_to_destination(dest, src, max_chars_count, module_name, __func__))
        {
            break;
        }

        memset(dest, '\0', max_chars_count);

        char* temp = kzalloc(max_chars_count, GFP_KERNEL);

        if (temp == NULL)
        {
            pr_err("%s: trim_and_copy_string : memory could not be allocated!\n", module_name);
            break;
        }

        memset(temp, '\0', max_chars_count);
        strncpy(temp, src, max_chars_count - 1);

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

void convert_to_same_case_and_copy_string(char* dest, const char* src, size_t max_chars_count, int to_lower_case,
                                          const char* calling_module_name)
{
    const char* module_name = calling_module_name ? calling_module_name : "INVALID MODULE NAME";

    if (can_copy_to_destination(dest, src, max_chars_count, module_name, __func__))
    {
        memset(dest, '\0', max_chars_count);

        if (to_lower_case)
        {
            for (size_t index = 0; index < strlen(src); ++index)
            {
                dest[index] = tolower(src[index]);
            }
        }
        else
        {
            for (size_t index = 0; index < strlen(src); ++index)
            {
                dest[index] = toupper(src[index]);
            }
        }
    }
}

void reverse_and_copy_string(char* dest, const char* src, size_t max_chars_count, const char* calling_module_name)
{
    const char* module_name = calling_module_name ? calling_module_name : "INVALID MODULE NAME";

    if (can_copy_to_destination(dest, src, max_chars_count, module_name, __func__))
    {
        memset(dest, '\0', max_chars_count);

        const size_t length = strlen(src);

        for (size_t index = 0; index < length; ++index)
        {
            dest[index] = src[length - 1 - index];
        }
    }
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
EXPORT_SYMBOL(convert_to_same_case_and_copy_string);
EXPORT_SYMBOL(reverse_and_copy_string);
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
