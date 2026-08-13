#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/mutex.h>

#include "kernel_mutex_impl.h"

static struct task_struct* sos_thread = NULL;
static struct task_struct* sms_thread = NULL;

static struct mutex message_mutex;

/***** THREADS FUNCTIONALITY *****/

static int display_sos(void* arg)
{
    while (!kthread_should_stop())
    {
        msleep(300);
        mutex_lock(&message_mutex);
        pr_alert("%s: S.O.S. thread: printing first: \"S.\"\n", THIS_MODULE->name);
        msleep(100);
        pr_alert("%s: S.O.S. thread: printing: \"O.\"\n", THIS_MODULE->name);
        msleep(100);
        pr_alert("%s: S.O.S. thread: printing second: \"S.\"\n", THIS_MODULE->name);
        mutex_unlock(&message_mutex);
        msleep(100);
    }

    return 0;
}

static int display_sms(void* arg)
{
    while (!kthread_should_stop())
    {
        msleep(300);
        mutex_lock(&message_mutex);
        pr_warn("%s: S.M.S. thread: printing first: \"S.\"\n", THIS_MODULE->name);
        msleep(100);
        pr_warn("%s: S.M.S. thread: printing: \"M.\"\n", THIS_MODULE->name);
        msleep(100);
        pr_warn("%s: S.M.S. thread: printing second: \"S.\"\n", THIS_MODULE->name);
        mutex_unlock(&message_mutex);
        msleep(100);
    }

    return 0;
}

static void toggle_thread(struct task_struct** thread, int (*thread_fn)(void*), const char* thread_name)
{
    if (thread && thread_fn && thread_name)
    {
        if (!*thread)
        {
            *thread = kthread_run(thread_fn, NULL, thread_name);

            if (!*thread)
            {
                pr_err("%s: cannot create %s!\n", THIS_MODULE->name, thread_name);
            }
        }
        else
        {
            kthread_stop(*thread);
            *thread = NULL;
        }
    }
    else
    {
        pr_err("%s: cannot toggle thread, at least one of the parameters is NULL!\n", THIS_MODULE->name);
    }
}

// by default only the "S.O.S." thread is started
void init_threads(void)
{
    mutex_init(&message_mutex);

    if (!sos_thread)
    {
        sos_thread = kthread_run(display_sos, NULL, "S.O.S. thread");

        if (!sos_thread)
        {
            pr_err("%s: cannot create S.O.S. thread!\n", THIS_MODULE->name);
        }
    }
    else
    {
        pr_warn("%s: S.O.S. thread is already running!\n", THIS_MODULE->name);
    }
}

void clear_threads(void)
{
    if (sos_thread)
    {
        kthread_stop(sos_thread);
        sos_thread = NULL;
    }

    if (sms_thread)
    {
        kthread_stop(sms_thread);
        sos_thread = NULL;
    }
}

/***** READ/WRITE IMPLEMENTATION FUNCTIONS *****/

// each read toggles emitting "S.O.S" on/off
ssize_t device_read_impl(struct file* filp, char* buf, size_t length, loff_t* offset)
{
    toggle_thread(&sos_thread, display_sos, "S.O.S. thread");
    return 0;
}

// each write toggles emitting "S.M.S" on/off
ssize_t device_write_impl(struct file* filp, const char* buf, size_t length, loff_t* offset)
{
    toggle_thread(&sms_thread, display_sms, "S.M.S. thread");
    return length;
}
