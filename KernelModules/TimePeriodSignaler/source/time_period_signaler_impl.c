#include <linux/module.h>

#include "time_period_signaler_impl.h"

#define SIGTIMEOUT 44
#define MILLISECONDS_IN_SECOND 1000
#define JIFFIES_IN_SECOND HZ

static struct task_struct* current_task = NULL;
static int signal_number = 0;

static struct timer_list timer;
static size_t timeout_period_ms = 1000; // default timeout period

static void reset(void)
{
    del_timer(&timer);

    current_task = NULL;
    signal_number = 0;
}

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
        reset();
    }
}

static void handle_timeout(struct timer_list* tm)
{
    trigger_timeout_signal();
    del_timer(tm);
}

static void start_timer(void)
{
    timer.expires = jiffies_64 + (JIFFIES_IN_SECOND * timeout_period_ms) / MILLISECONDS_IN_SECOND;
    timer_setup(&timer, handle_timeout, 0);

    add_timer(&timer);
}

long ioctl_request_signal(struct task_struct* task, size_t* timeout_period_data)
{
    long result = -1;

    do
    {
        // only a task can be registered at a time; timeout will unregister it automatically
        if (!task || !timeout_period_data || current_task)
        {
            break;
        }

        size_t timeout_period;
        size_t bytes_not_copied_count = copy_from_user(&timeout_period, timeout_period_data, sizeof(timeout_period));

        if (bytes_not_copied_count > 0)
        {
            break;
        }

        pr_info("%s: ioctl_request_signal(): requested timeout signal %d\n", THIS_MODULE->name, SIGTIMEOUT);

        current_task = task;
        timeout_period_ms = timeout_period;
        signal_number = SIGTIMEOUT;
        result = 0;

        start_timer();
    } while (false);

    return result;
}

long ioctl_reset(struct task_struct* task)
{
    long result = -1;

    // reset can only be issued by the currently registered task
    if (task && task == current_task)
    {
        pr_info("%s: ioctl_reset(): reset requested\n", THIS_MODULE->name);

        reset();
        result = 0;
    }

    return result;
}
