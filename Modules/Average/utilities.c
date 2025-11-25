#include <linux/module.h>

MODULE_LICENSE("GPL");

uint get_string_length(const char *str)
{
    uint len = 0;

    if (str != NULL)
    {
        while (str[len] != '\0')
        {
            ++len;
        }
    }

    return len;
}

int is_empty_string(const char *str)
{
    return 0 == get_string_length(str);
}

int get_average(const int *array, uint arraySize)
{
    int sum = 0;

    for (uint index = 0; index < arraySize; ++index)
    {
        sum += array[index];
    }

    return arraySize > 0 ? sum / (int)arraySize : sum;
}
