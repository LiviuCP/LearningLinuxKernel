#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#define INT_ARRAY_SZ 10

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Liviu Popa");
MODULE_DESCRIPTION("This module calculates a rounded average of up to 10 integer values provided by user. Minimum 2 "
                   "integers are required.");

static int intArray[INT_ARRAY_SZ];
static uint averagedElementsCount = 0;

extern int print_array_elements_to_be_averaged(const int *, uint);
extern int get_average(const int *, uint);

module_param_array(intArray, int, &averagedElementsCount, S_IRUSR | S_IWUSR);
MODULE_PARM_DESC(intArray, " an array of integers that are supposed to be averaged");

static const uint minAveragedParamsCount = 2;

static int __init average2_init(void)
{
    int result = -1;

    pr_info("%s: this module is supposed to calculate a rounded average of array integers!\n", THIS_MODULE->name);
    pr_info("%s: maximum count of integers is: %ld\n", THIS_MODULE->name, ARRAY_SIZE(intArray));
    pr_info("%s: elements count to be averaged is calculated internally\n", THIS_MODULE->name);

    if (averagedElementsCount >= minAveragedParamsCount)
    {
        result = 0;
        print_array_elements_to_be_averaged(intArray, averagedElementsCount);
        pr_info("%s: the average is: %d\n", THIS_MODULE->name, get_average(intArray, averagedElementsCount));
    }
    else
    {
        pr_err("%s: invalid averaged elements count (%d)! There should be at least %d elements to be averaged!\n",
               THIS_MODULE->name, averagedElementsCount, minAveragedParamsCount);
    }

    return result;
}

module_init(average2_init);
