#include <linux/module.h>

#include "time_period_signaler_impl.h"

// TODO: implement
long ioctl_request_signal(void)
{
    pr_info("%s: ioctl_request_signal() - under construction!\n", THIS_MODULE->name);
    return 0;
}

// TODO: implement
long ioctl_reset(void)
{
    pr_info("%s: ioctl_reset() - under construction!\n", THIS_MODULE->name);
    return 0;
}
