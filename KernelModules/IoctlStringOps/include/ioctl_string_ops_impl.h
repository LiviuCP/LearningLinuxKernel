#pragma once

#define SUCCESS 0
#define BUFFER_SIZE 1024
#define SUPPORTED_MINOR_NUMBERS_COUNT 1

#define TRIM_USER_INPUT_ENABLED 0b00000001
#define OUTPUT_PREFIX_ENABLED 0b00000010
#define DEFAULT_SETTINGS 0b00000001

void ioctl_get_buffer_size(char* module_buffer, size_t* buffer_size);
void ioctl_trim_user_input(uint8_t* settings, uint8_t* should_trim);
void ioctl_do_module_reset(char* module_buffer, uint8_t* settings);
void ioctl_is_module_reset(char* module_buffer, uint8_t* settings, uint8_t* is_module_reset);
