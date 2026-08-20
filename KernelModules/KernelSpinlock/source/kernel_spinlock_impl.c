#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/random.h>

#include "kernel_spinlock_impl.h"

static struct task_struct* first_worker_thread = NULL;
static struct task_struct* second_worker_thread = NULL;
static struct task_struct* monitoring_thread = NULL;

static spinlock_t lock;

/***** MEASUREMENT VARIABLES *****/

// first worker thread increases this value, second worker thread decreases it
static int shared_value = 0;

static size_t worker_threads_lock_aquires_count = 0;
static size_t first_worker_thread_successful_trylocks_count = 0;
static size_t first_worker_thread_failed_trylocks_count = 0;
static size_t second_worker_thread_successful_trylocks_count = 0;
static size_t second_worker_thread_failed_trylocks_count = 0;
static size_t monitoring_thread_measured_lock_aquires_count = 0;

/***** THREADS FUNCTIONALITY *****/

static uint32_t get_random_sleep_period_ms(void)
{
    static const uint32_t min_period = 96;
    static const uint32_t max_interval_value = 5;
    return min_period + get_random_u32() % max_interval_value;
}

// emulates some arbitrary work that would take a little longer
static void emulate_additional_work(void)
{
    static const uint32_t zero_val = 0;
    static const uint32_t large_constant = ~zero_val;

    for (uint32_t row = 0; row < large_constant; ++row)
    {
        for (uint32_t column = 0; column < large_constant; ++column)
        {
            --shared_value;
            ++shared_value;
        }
    }
}

static int first_worker_thread_fn(void* arg)
{
    while (!kthread_should_stop())
    {
        const uint32_t sleep_period_ms = get_random_sleep_period_ms();
        msleep(sleep_period_ms);

        const int trylock_result = spin_trylock(&lock);

        // try lock just for checking if a thread already holds the lock, if not lock anyway
        if (trylock_result == 0)
        {
            ++first_worker_thread_failed_trylocks_count;
            spin_lock(&lock);
        }
        else
        {
            ++first_worker_thread_successful_trylocks_count;
        }

        ++shared_value;
        ++worker_threads_lock_aquires_count;
        pr_info("%s: FIRST WORKER thread: incremented shared value to %d\n", THIS_MODULE->name, shared_value);
        emulate_additional_work();
        spin_unlock(&lock);
    }

    return 0;
}

static int second_worker_thread_fn(void* arg)
{
    while (!kthread_should_stop())
    {
        const uint32_t sleep_period_ms = get_random_sleep_period_ms();
        msleep(sleep_period_ms);

        const int trylock_result = spin_trylock(&lock);

        // try lock just for checking if a thread already holds the lock, if not lock anyway
        if (trylock_result == 0)
        {
            ++second_worker_thread_failed_trylocks_count;
            spin_lock(&lock);
        }
        else
        {
            ++second_worker_thread_successful_trylocks_count;
        }

        --shared_value;
        ++worker_threads_lock_aquires_count;
        pr_info("%s: SECOND WORKER thread: decremented shared value to %d\n", THIS_MODULE->name, shared_value);
        emulate_additional_work();
        spin_unlock(&lock);
    }

    return 0;
}

/* The monitoring thread measures the number of times the lock is aquired.
   The resulting value should be the number of times first two threads aquired the lock.

   For maximum accuracy the number of transitions from unlocked to locked
   is being measured.

   Note: some small differences to the expected result have been observed, i.e. the total
   number of aquires is less than the sum of the aquires of the two worker threads.

   This is caused by two factors:
   - a thread waits for the lock release before aquiring it
   - a thread immediately aquires the lock (without waiting) once the other thread released it ("perfect match")

   In both cases the third thread might see a single "lock aquired" time slot in which actually both threads did their
   work.
 */
static int monitoring_thread_fn(void* arg)
{
    int current_lock_status = 0; // unlocked

    while (!kthread_should_stop())
    {
        const int lock_result = spin_is_locked(&lock);

        if (lock_result == 0)
        {
            if (current_lock_status != 0) // locked (aquired)
            {
                current_lock_status = lock_result;
            }

            continue;
        }

        if (current_lock_status == 0)
        {
            current_lock_status = lock_result;
            ++monitoring_thread_measured_lock_aquires_count;
        }
    }

    return 0;
}

static void init_thread(struct task_struct** thread, int (*thread_fn)(void*), const char* thread_name)
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
            pr_warn("%s: %s is already running!\n", THIS_MODULE->name, thread_name);
        }
    }
    else
    {
        pr_err("%s: cannot initialize thread, at least one of the parameters is NULL!\n", THIS_MODULE->name);
    }
}

static void clear_thread(struct task_struct** thread)
{
    if (thread && *thread)
    {
        kthread_stop(*thread);
        *thread = NULL;
    }
}

void init_threads(void)
{
    spin_lock_init(&lock);

    init_thread(&first_worker_thread, first_worker_thread_fn, "FIRST WORKER thread");
    init_thread(&second_worker_thread, second_worker_thread_fn, "SECOND WORKER thread");
    init_thread(&monitoring_thread, monitoring_thread_fn, "MONITORING thread");
}

void clear_threads(void)
{
    clear_thread(&first_worker_thread);
    clear_thread(&second_worker_thread);
    clear_thread(&monitoring_thread);

    pr_info("%s: ---- MODULE SUMMARY ----: \n", THIS_MODULE->name);
    pr_info("%s: FIRST and SECOND WORKER threads aquired the lock %d times\n", THIS_MODULE->name,
            (int)worker_threads_lock_aquires_count);
    pr_info("%s: FIRST WORKER thread found the lock free %d times and waited for it %d times\n", THIS_MODULE->name,
            (int)first_worker_thread_successful_trylocks_count, (int)first_worker_thread_failed_trylocks_count);
    pr_info("%s: SECOND WORKER thread found the lock free %d times and waited for it %d times\n", THIS_MODULE->name,
            (int)second_worker_thread_successful_trylocks_count, (int)second_worker_thread_failed_trylocks_count);
    pr_info("%s: the value of FIRST and SECOND thread shared variable is %d\n", THIS_MODULE->name, shared_value);
    pr_info("%s: MONITORING thread found the lock aquired %d times\n", THIS_MODULE->name,
            (int)monitoring_thread_measured_lock_aquires_count);

    worker_threads_lock_aquires_count = 0;
    first_worker_thread_successful_trylocks_count = 0;
    first_worker_thread_failed_trylocks_count = 0;
    second_worker_thread_successful_trylocks_count = 0;
    second_worker_thread_failed_trylocks_count = 0;
    monitoring_thread_measured_lock_aquires_count = 0;
}
