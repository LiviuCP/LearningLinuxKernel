#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include "Common/average_utilities.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Liviu Popa");
MODULE_DESCRIPTION("This module calculates a rounded average of up to 10 integer values provided by user/contained "
                   "within integers array. The number of integers to be averaged should be explicitly passed as "
                   "parameters. Minimum 2 integers are required.");

static int int_array[INT_ARRAY_SIZE];
static uint averaged_elements_count = 0;

module_param_array(int_array, int, NULL, S_IRUSR | S_IWUSR);
MODULE_PARM_DESC(int_array, " an array of integers that are supposed to be averaged");

module_param(averaged_elements_count, uint, S_IRUSR | S_IWUSR);
MODULE_PARM_DESC(averaged_elements_count,
                 " user provided number of elements to be averaged (should not exceed the intrinsic array size "
                 "INT_ARRAY_SIZE; in case it exceeds the user provided values count the extra values will be 0)");

static const uint min_averaged_params_count = 2;

static int __init average1_init(void)
{
    int result = -1;

    pr_info("%s: this module is supposed to calculate a rounded average of array integers!\n", THIS_MODULE->name);
    pr_info("%s: maximum count of integers is: %ld\n", THIS_MODULE->name, ARRAY_SIZE(int_array));
    pr_info("%s: elements count to be averaged is provided as argument\n", THIS_MODULE->name);

    if (averaged_elements_count < min_averaged_params_count)
    {
        pr_err("%s: invalid averaged elements count (%d)! There should be at least %d elements to be averaged!\n",
               THIS_MODULE->name, averaged_elements_count, min_averaged_params_count);
    }
    else if (averaged_elements_count > ARRAY_SIZE(int_array))
    {
        pr_err("%s: invalid averaged elements count (%d)! There should be at most %ld elements to be averaged!\n",
               THIS_MODULE->name, averaged_elements_count, ARRAY_SIZE(int_array));
    }
    else
    {
        result = 0;
        print_array_elements_to_be_averaged(int_array, averaged_elements_count);
        pr_info("%s: the average is: %d\n", THIS_MODULE->name, get_average(int_array, averaged_elements_count));
    }

    return result;
}

module_init(average1_init);
