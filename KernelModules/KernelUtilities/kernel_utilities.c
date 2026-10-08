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

void convert_to_same_case_and_copy_string(char* dest, const char* src, size_t max_chars_count, bool to_lower_case,
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

static size_t get_alphanumeric_chars_count(const char* src)
{
    size_t alnum_chars_count = 0;

    if (src)
    {
        for (size_t index = 0; index < strlen(src); ++index)
        {
            if (isalnum(src[index]))
            {
                ++alnum_chars_count;
            }
        }
    }

    return alnum_chars_count;
}

/* This function formats the source string and copies the resulting content into a destination string.
   Formatting is performed as follows:
   - clean up any character that is not alphabetic or digit
   - split the remaining characters into groups of 4, add '_' between groups
   - if the last group has less than 4 characters following corner cases apply:
     a) 3 characters:
        - if there is a preceding group a '-' character is appended at the end
        - if there is no preceding group no character is appended
     b) 2 characters:
        - if there is a preceding group the last character is taken from it and prepended to the last group
        - if there is no preceding group then a '-' character is appended
     c) 1 character:
        - if there is a preceding group the last character is taken from it and prepended to the last group; a '-'
   character is also appended
        - if there is no preceding group then two '-' characters are appended
     d) 0 characters:
        - if the input string has no characters then three '-' characters are added
   - goal:
        - there should be minimum 3 characters in the resulting sequence / each group
        - last two groups (if existing) should have the same number of characters
*/
void format_and_copy_string(char* dest, const char* src, size_t max_chars_count, const char* calling_module_name)
{
    const char* module_name = calling_module_name ? calling_module_name : "INVALID MODULE NAME";

    do
    {
        // minimum requirements are related to "plain copy" operation (necessary, not sufficient, see below)
        if (!can_copy_to_destination(dest, src, max_chars_count, module_name, __func__))
        {
            break;
        }

        const size_t alnum_chars_count = get_alphanumeric_chars_count(src);
        const size_t group_size = 4;
        const size_t whole_groups_count = alnum_chars_count / group_size;
        const size_t residual_chars_count = alnum_chars_count % group_size;
        const bool has_residual_chars = residual_chars_count > 0;
        const size_t underscores_count =
            whole_groups_count > 0 || has_residual_chars ? whole_groups_count + (size_t)has_residual_chars - 1 : 0;
        const size_t minus_chars_count = residual_chars_count == 1   ? (whole_groups_count > 0 ? 1 : 2)
                                         : residual_chars_count == 2 ? (whole_groups_count > 0 ? 0 : 1)
                                         : residual_chars_count == 3 ? (whole_groups_count > 0 ? 1 : 0)
                                         : alnum_chars_count == 0    ? 3
                                                                     : 0;

        const size_t total_chars_count = alnum_chars_count + underscores_count + minus_chars_count;

        // max_chars_count includes the terminating '\0' character
        if (total_chars_count >= max_chars_count)
        {
            pr_warn("%s: %s: src length exceeds maximum dest string length!\n", module_name, __func__);
            break;
        }

        memset(dest, '\0', max_chars_count);

        const size_t src_length = strlen(src);

        if (src_length == 0)
        {
            strncpy(dest, "---", 3);
            break;
        }

        size_t current_src_index = 0;
        size_t current_dest_index = 0;
        size_t current_group_index = 0; // relative index within group of chars

        // step 1: copy the groups that are not subject to change
        if (whole_groups_count > 1)
        {
            const size_t groups_to_copy_count = whole_groups_count - 1;
            size_t copied_groups_count = 0;

            while (current_src_index < src_length && copied_groups_count < groups_to_copy_count)
            {
                if (!isalnum(src[current_src_index]))
                {
                    ++current_src_index;
                    continue;
                }

                dest[current_dest_index] = src[current_src_index];
                ++current_dest_index;
                ++current_src_index;
                ++current_group_index;

                if (current_group_index == 4)
                {
                    current_group_index = 0;
                    dest[current_dest_index] = '_';
                    ++current_dest_index;
                    ++copied_groups_count;
                }
            }
        }

        const size_t last_non_residual_chars_to_copy_count =
            whole_groups_count > 0 ? residual_chars_count == 1 || residual_chars_count == 2 ? 3 : 4 : 0;

        const size_t chars_to_take_from_last_whole_group_count =
            whole_groups_count > 0 ? residual_chars_count == 1 || residual_chars_count == 2 ? 1 : 0 : 0;

        current_group_index = 0; // defensive programming (should have already been set to 0, see above)

        // step 2: copy the last full group (minus number of characters to be moved to residual group)
        while (current_src_index < src_length && current_group_index < last_non_residual_chars_to_copy_count)
        {
            if (!isalnum(src[current_src_index]))
            {
                ++current_src_index;
                continue;
            }

            dest[current_dest_index] = src[current_src_index];
            ++current_dest_index;
            ++current_src_index;
            ++current_group_index;
        }

        if (!has_residual_chars)
        {
            break;
        }

        if (whole_groups_count > 0)
        {
            dest[current_dest_index] = '_';
            ++current_dest_index;
        }

        current_group_index = 0;

        // step 3: copy the residual group (including any moved characters from previous group)
        while (current_src_index < src_length &&
               current_group_index < chars_to_take_from_last_whole_group_count + residual_chars_count)
        {
            if (!isalnum(src[current_src_index]))
            {
                ++current_src_index;
                continue;
            }

            dest[current_dest_index] = src[current_src_index];
            ++current_dest_index;
            ++current_src_index;
            ++current_group_index;
        }

        // step 4: add padding chars ('-')
        for (size_t index = 0; index < minus_chars_count; ++index)
        {
            dest[current_dest_index] = '-';
            ++current_dest_index;
        }
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
EXPORT_SYMBOL(convert_to_same_case_and_copy_string);
EXPORT_SYMBOL(reverse_and_copy_string);
EXPORT_SYMBOL(format_and_copy_string);
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
