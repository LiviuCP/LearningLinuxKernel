#pragma once

ssize_t device_read_impl(struct file* filp, char* buf, size_t length, loff_t* offset);
ssize_t device_write_impl(struct file* filp, const char* buf, size_t length, loff_t* offset);

long ioctl_do_module_reset(void);
long ioctl_is_module_reset(bool* is_module_reset);
long ioctl_enable_user_input_trimming(const bool* should_trim);
long ioctl_is_user_input_trimming_enabled(bool* is_trimming_enabled);
long ioctl_get_buffer_size(size_t* buffer_size);
long ioctl_set_output_prefix(const void* output_prefix_data);
long ioctl_get_output_prefix_size(size_t* output_prefix_size);
long ioctl_enable_input_append_mode(const bool* should_append);
long ioctl_is_input_append_mode_enabled(bool* is_append_enabled);

/* The value input by user is the maximum number of bytes to read from data buffer
   The value written back by module is the number of characters left to read from data buffer
   (output prefix excluded)
*/
long ioctl_set_max_output_size(size_t* value);

long ioctl_get_max_output_size(size_t* max_output_length);

void reset_module_data(void);
