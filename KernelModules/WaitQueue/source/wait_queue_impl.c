#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/wait.h>

#include "wait_queue_impl.h"

// small buffer sizes used for easy testing
#define INPUT_BUFFER_SIZE 128
#define BUFFER_SIZE 256

static char
    input_buffer[INPUT_BUFFER_SIZE]; // buffer where input is stored before formatting and writing to data buffer
static char buffer[BUFFER_SIZE];     // data buffer

static wait_queue_head_t wait_queue;
static struct task_struct* formatting_thread = NULL;
static bool is_exit_requested = false;

// min nr of chars from input buffer that trigger a formatting operation
static const size_t input_formatting_threshold = INPUT_BUFFER_SIZE / 3;

extern void format_and_copy_string(char* dest, const char* src, size_t max_chars_count,
                                   const char* calling_module_name);

/***** HELPER FUNCTIONS *****/

static bool can_process_wakeup_event(void)
{
    const bool is_formatting_threshold_reached = strlen(input_buffer) >= input_formatting_threshold;
    const bool is_output_buffer_empty = strlen(buffer) == 0;

    return (is_formatting_threshold_reached && is_output_buffer_empty) || is_exit_requested;
}

static int formatting_thread_function(void* unused)
{
    for (;;)
    {

        pr_info("%s: waiting for queue event\n", THIS_MODULE->name);
        wait_event_interruptible(wait_queue, can_process_wakeup_event());

        if (is_exit_requested)
        {
            // not necessarily required, just a "precaution"
            reset_module_data();

            break;
        }

        format_and_copy_string(buffer, input_buffer, BUFFER_SIZE, THIS_MODULE->name);
        pr_info("%s: Formatted string \"%s\" is available in the output buffer\n", THIS_MODULE->name, buffer);

        // input formatting clears the input buffer
        memset(input_buffer, '\0', INPUT_BUFFER_SIZE);
    }

    return 0;
}

/***** INIT/EXIT *****/

bool init_queue()
{
    init_waitqueue_head(&wait_queue);

    formatting_thread = kthread_create(formatting_thread_function, NULL, "FormattingThread");

    if (formatting_thread)
    {
        wake_up_process(formatting_thread);
    }
    else
    {
        pr_err("%s: unable to create formatting thread\n", THIS_MODULE->name);
    }

    return formatting_thread != NULL;
}

void clear_queue()
{
    wake_up_interruptible(&wait_queue);
}

/***** READ/WRITE IMPLEMENTATION FUNCTIONS *****/

ssize_t device_read_impl(struct file* filp, char* buf, size_t length, loff_t* offset)
{
    const char* buffer_ptr = buffer;

    ssize_t read_bytes_count = 0;
    const size_t max_bytes_to_read_count = BUFFER_SIZE - 1;

    size_t bytes_to_read_count = length > max_bytes_to_read_count ? max_bytes_to_read_count : length;

    while (bytes_to_read_count && *buffer_ptr)
    {
        put_user(*(buffer_ptr++), buf++);
        ++read_bytes_count;
        --bytes_to_read_count;
    }

    if (read_bytes_count > 0)
    {
        pr_info("%s: user read: %s", THIS_MODULE->name, buffer);
    }

    // clear the output buffer once the user has read the data
    memset(buffer, '\0', BUFFER_SIZE);

    return read_bytes_count;
}

ssize_t device_write_impl(struct file* filp, const char* buf, size_t length, loff_t* offset)
{
    ssize_t result = -EINVAL;

    do
    {
        const size_t input_buffer_length = strlen(input_buffer);

        // terminating '\0' needs to be taken into account
        const size_t available_capacity = INPUT_BUFFER_SIZE - 1 - input_buffer_length;

        if (length > available_capacity)
        {
            pr_err("%s: not enough space in input buffer to copy from raw input buffer!", THIS_MODULE->name);
            break;
        }

        char* raw_input_buffer = kzalloc(INPUT_BUFFER_SIZE, GFP_KERNEL);

        if (!raw_input_buffer)
        {
            pr_err("%s: device_write_impl: memory could not be allocated!\n", THIS_MODULE->name);
            result = -ENOMEM;
            break;
        }

        memset(raw_input_buffer, '\0', INPUT_BUFFER_SIZE);

        const size_t bytes_not_copied_count = copy_from_user(raw_input_buffer, buf, length);

        if (bytes_not_copied_count > 0)
        {
            pr_warn("%s: %ld bytes could not be copied from user\n", THIS_MODULE->name, bytes_not_copied_count);
        }

        pr_info("%s: user wrote: %s\n", THIS_MODULE->name, raw_input_buffer);

        const size_t copied_chars_count = strlen(raw_input_buffer);

        strncpy(input_buffer + input_buffer_length, raw_input_buffer, copied_chars_count);

        result = (ssize_t)strlen(
            raw_input_buffer); // the total number of chars provided by user (not the trimmed one) needs to be returned

        kfree(raw_input_buffer);
    } while (false);

    return result;
}

/***** RELEASE/RESET *****/

// For each input/output operation the thread is woken up.
// However a formatting will only be performed if both conditions are valid:
// - the characters from input buffer reached the given threshold (currently 1/3 of the buffer capacity)
// - the output (data) buffer is empty
void handle_post_device_release_actions(void)
{
    wake_up_interruptible(&wait_queue);
}

void reset_module_data(void)
{
    memset(input_buffer, '\0', INPUT_BUFFER_SIZE);
    memset(buffer, '\0', BUFFER_SIZE);
}
