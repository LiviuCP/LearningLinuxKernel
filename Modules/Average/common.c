#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

void print_array_elements_to_be_averaged(const int* array, uint arraySize)
{
    if (array != NULL)
    {
        pr_info("%s: %d elements to be averaged\n", THIS_MODULE->name, arraySize);
        pr_info("%s: the elements to be averaged are:\n", THIS_MODULE->name);

        for (uint index = 0; index < arraySize; ++index)
        {
            pr_info("%s: intArray[%d] = %d\n", THIS_MODULE->name, index, array[index]);
        }
    }
}
