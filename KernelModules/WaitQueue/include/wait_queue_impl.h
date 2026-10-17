#pragma once

bool init_queue(void);
void clear_queue(void);

ssize_t device_read_impl(struct file* filp, char* buf, size_t length, loff_t* offset);
ssize_t device_write_impl(struct file* filp, const char* buf, size_t length, loff_t* offset);

void handle_post_device_release_actions(void);

void reset_module_data(void);
