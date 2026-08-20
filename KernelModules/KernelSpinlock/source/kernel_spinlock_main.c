#include <linux/module.h>

#include "kernel_spinlock_impl.h"

MODULE_LICENSE("GPL");

MODULE_DESCRIPTION("This driver module illustrates kernel threads and kernel spinlock.\n"
                   "Three threads are used: 2 worker threads and 1 monitoring thread.\n"
                   "First worker thread increments a value.\n"
                   "Second worker thread decrements the same value.\n"
                   "The monitoring thread checks whether the spin lock is free or not without manipulating it.\n"
                   "Any running thread is stopped by the exit function.\n");

MODULE_AUTHOR("Liviu Popa");

static int kernel_spinlock_init(void)
{
    init_threads();
    return 0;
}

static void kernel_spinlock_exit(void)
{
    clear_threads();
}

module_init(kernel_spinlock_init);
module_exit(kernel_spinlock_exit);
