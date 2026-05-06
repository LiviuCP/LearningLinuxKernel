#pragma once

ssize_t device_read_impl(struct file* filp, char* buf, size_t length, loff_t* offset);
ssize_t device_write_impl(struct file* filp, const char* buf, size_t length, loff_t* offset);

long ioctl_do_module_reset(void);
long ioctl_is_module_reset(uint8_t* is_module_reset);
long ioctl_trim_user_input(const uint8_t* should_trim);
long ioctl_get_chars_count_from_buffer(size_t* buffer_size);
long ioctl_set_output_prefix(const void* output_prefix_data);
long ioctl_get_output_prefix_size(size_t* output_prefix_size);
long ioctl_enable_input_append_mode(uint8_t* should_append);
long ioctl_is_input_append_mode_enabled(uint8_t* is_append_enabled);

void reset_buffers(void);
