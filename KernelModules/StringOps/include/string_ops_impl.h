#pragma once

#define SUCCESS 0
#define INPUT_BUFFER_SIZE 128
#define DATA_BUFFER_SIZE 256
#define SUPPORTED_MINOR_NUMBERS_COUNT 4
#define TO_LOWER_CASE true

ssize_t device_read_impl(struct file* filp, char* buffer, size_t length, loff_t* offset, int minor_number);
ssize_t device_write_impl(struct file* filp, const char* buffer, size_t length, loff_t* offset, int minor_number);

void link_minor_number_data(int minor_number);
bool is_valid_minor_number(int minor_number);

void reset_module_data(void);
