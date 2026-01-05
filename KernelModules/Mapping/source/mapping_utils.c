#include <linux/ctype.h>
#include <linux/module.h>

#include "mapping_utils.h"

MODULE_LICENSE("GPL");

void trim_and_copy_str(char* dest, const char* src, size_t max_str_length)
{
    do
    {
        if (!dest)
        {
            pr_warn("%s: NULL dest string!\n", THIS_MODULE->name);
            break;
        }

        if (!src)
        {
            pr_warn("%s: NULL src string!\n", THIS_MODULE->name);
            break;
        }

        memset(dest, '\0', max_str_length);

        char temp[max_str_length];

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
    } while (false);
}
