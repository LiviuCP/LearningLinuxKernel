#include <linux/module.h>

#include "time_period_signaler_impl.h"

#define SIGTIMEOUT 44

static struct task_struct* current_task = NULL;
static int signal_number = 0;

static void trigger_timeout_signal(void)
{
    if (current_task && signal_number == SIGTIMEOUT)
    {
        struct kernel_siginfo info;
        memset(&info, 0, sizeof(info));

        info.si_signo = signal_number;
        info.si_code = SI_QUEUE;
        info.si_int = 1;

        pr_info("%s: trigger_timeout_signal(): sending signal %d to process\n", THIS_MODULE->name, info.si_signo);

        if (send_sig_info(SIGTIMEOUT, &info, current_task) < 0)
        {
            pr_err("%s: unable to send signal %d!\n", THIS_MODULE->name, info.si_signo);
        }
        else
        {
            pr_info("%s: successfully sent signal %d!\n", THIS_MODULE->name, info.si_signo);
        }

        // unregister current task
        ioctl_reset();
    }
}

long ioctl_request_signal(struct task_struct* task)
{
    long result = -1;

    if (task && !current_task)
    {
        current_task = task;
        signal_number = SIGTIMEOUT;

        trigger_timeout_signal();

        result = 0;
    }

    return result;
}

long ioctl_reset(void)
{
    current_task = NULL;
    signal_number = 0;

    return 0;
}
