#include <linux/module.h>

#include "Common/average_utilities.h"

MODULE_LICENSE("GPL");

void print_array_elements_to_be_averaged(const int* array, uint array_size)
{
    if (array != NULL)
    {
        pr_info("%s: %d elements to be averaged\n", THIS_MODULE->name, array_size);
        pr_info("%s: the elements to be averaged are:\n", THIS_MODULE->name);

        for (uint index = 0; index < array_size; ++index)
        {
            pr_info("%s: intArray[%d] = %d\n", THIS_MODULE->name, index, array[index]);
        }
    }
}

int get_average(const int* array, uint array_size)
{
    int sum = 0;

    for (uint index = 0; index < array_size; ++index)
    {
        sum += array[index];
    }

    return array_size > 0 ? sum / (int)array_size : sum;
}
