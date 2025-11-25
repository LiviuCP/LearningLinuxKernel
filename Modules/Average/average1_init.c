#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#define INT_ARRAY_SZ 10

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Liviu Popa");
MODULE_DESCRIPTION("This module calculates a rounded average of up to 10 integer values provided by user/contained "
                   "within integers array. The number of integers to be averaged should be explicitly passed as "
                   "parameters. Minimum 2 integers are required.");

static int intArray[INT_ARRAY_SZ];
static uint averagedElementsCount = 0;

extern int print_array_elements_to_be_averaged(const int*, uint);
extern int get_average(const int*, uint);

module_param_array(intArray, int, NULL, S_IRUSR | S_IWUSR);
MODULE_PARM_DESC(intArray, " an array of integers that are supposed to be averaged");

module_param(averagedElementsCount, uint, S_IRUSR | S_IWUSR);
MODULE_PARM_DESC(averagedElementsCount,
                 " user provided number of elements to be averaged (should not exceed the intrinsic array size "
                 "INT_ARRAY_SZ; in case it exceeds the user provided values count the extra values will be 0)");

static const uint minAveragedParamsCount = 2;

static int __init average1_init(void)
{
    int result = -1;

    pr_info("%s: this module is supposed to calculate a rounded average of array integers!\n", THIS_MODULE->name);
    pr_info("%s: maximum count of integers is: %ld\n", THIS_MODULE->name, ARRAY_SIZE(intArray));
    pr_info("%s: elements count to be averaged is provided as argument\n", THIS_MODULE->name);

    if (averagedElementsCount < minAveragedParamsCount)
    {
        pr_err("%s: invalid averaged elements count (%d)! There should be at least %d elements to be averaged!\n",
               THIS_MODULE->name, averagedElementsCount, minAveragedParamsCount);
    }
    else if (averagedElementsCount > ARRAY_SIZE(intArray))
    {
        pr_err("%s: invalid averaged elements count (%d)! There should be at most %ld elements to be averaged!\n",
               THIS_MODULE->name, averagedElementsCount, ARRAY_SIZE(intArray));
    }
    else
    {
        result = 0;
        print_array_elements_to_be_averaged(intArray, averagedElementsCount);
        pr_info("%s: the average is: %d\n", THIS_MODULE->name, get_average(intArray, averagedElementsCount));
    }

    return result;
}

module_init(average1_init);
