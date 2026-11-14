#pragma once

long ioctl_request_timeout_signal(struct task_struct* task, size_t* timeout_period_data);
long ioctl_cancel_timeout_signal_request(struct task_struct* task);
