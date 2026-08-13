#pragma once

void init_threads(void);
void clear_threads(void);

ssize_t device_read_impl(struct file* filp, char* buf, size_t length, loff_t* offset);
ssize_t device_write_impl(struct file* filp, const char* buf, size_t length, loff_t* offset);
