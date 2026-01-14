#pragma once

#include <linux/ctype.h>
#include <linux/module.h>
#include <linux/slab.h>

static void trim_and_copy_string(char* dest, const char* src, size_t max_str_length)
{
    do
    {
        if (max_str_length == 0)
        {
            pr_warn("%s: invalid maximum string length!\n", THIS_MODULE->name);
            break;
        }

        if (!dest)
        {
            pr_warn("%s: NULL dest string!\n", THIS_MODULE->name);
            break;
        }

        memset(dest, '\0', max_str_length);

        if (!src)
        {
            pr_warn("%s: NULL src string!\n", THIS_MODULE->name);
            break;
        }

        char* temp = kzalloc(max_str_length, GFP_KERNEL);

        if (temp == NULL)
        {
            pr_err("%s: memory could not be allocated!\n", THIS_MODULE->name);
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
