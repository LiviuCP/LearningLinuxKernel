#pragma once

long ioctl_request_signal(struct task_struct* task, size_t* timeout_period_data);
long ioctl_reset(struct task_struct* task);
