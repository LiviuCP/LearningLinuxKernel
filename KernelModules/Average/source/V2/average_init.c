#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include "Common/average_utilities.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Liviu Popa");
MODULE_DESCRIPTION("This module calculates a rounded average of up to 10 integer values provided by user. Minimum 2 "
                   "integers are required.");

static int int_array[INT_ARRAY_SIZE];
static uint averaged_elements_count = 0;

module_param_array(int_array, int, &averaged_elements_count, S_IRUSR | S_IWUSR);
MODULE_PARM_DESC(int_array, " an array of integers that are supposed to be averaged");

static const uint min_averaged_params_count = 2;

static int __init average2_init(void)
{
    int result = -1;

    pr_info("%s: this module is supposed to calculate a rounded average of array integers!\n", THIS_MODULE->name);
    pr_info("%s: maximum count of integers is: %ld\n", THIS_MODULE->name, ARRAY_SIZE(int_array));
    pr_info("%s: elements count to be averaged is calculated internally\n", THIS_MODULE->name);

    if (averaged_elements_count >= min_averaged_params_count)
    {
        result = 0;
        print_array_elements_to_be_averaged(int_array, averaged_elements_count);
        pr_info("%s: the average is: %d\n", THIS_MODULE->name, get_average(int_array, averaged_elements_count));
    }
    else
    {
        pr_err("%s: invalid averaged elements count (%d)! There should be at least %d elements to be averaged!\n",
               THIS_MODULE->name, averaged_elements_count, min_averaged_params_count);
    }

    return result;
}

module_init(average2_init);
