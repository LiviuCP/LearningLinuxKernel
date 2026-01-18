#include <linux/module.h>

#include "Common/average_utilities.h"

MODULE_LICENSE("GPL");

void print_array_elements_to_be_averaged(const int* array, size_t array_size)
{
    if (array != NULL)
    {
        pr_info("%s: %ld elements to be averaged\n", THIS_MODULE->name, array_size);
        pr_info("%s: the elements to be averaged are:\n", THIS_MODULE->name);

        for (size_t index = 0; index < array_size; ++index)
        {
            pr_info("%s: intArray[%ld] = %d\n", THIS_MODULE->name, index, array[index]);
        }
    }
}
