#pragma once

#define SUCCESS 0
#define BUFFER_SIZE 1024
#define PREFIX_BUFFER_SIZE 128
#define SUPPORTED_MINOR_NUMBERS_COUNT 1

// 9999 is an arbitrarily chosen "magic number" (in a "real" (production) system an official assignment would be
// required; might be the major driver number)
#define IOCTL_TRIM_USER_INPUT _IOW(9999, 'a', uint8_t*)
#define IOCTL_GET_CHARS_COUNT_FROM_BUFFER _IOR(9999, 'b', size_t*)
#define IOCTL_SET_OUTPUT_PREFIX _IOW(9999, 'c', void*)
#define IOCTL_GET_OUTPUT_PREFIX_SIZE _IOR(9999, 'd', size_t*)
#define IOCTL_DO_MODULE_RESET _IOW(9999, 'e', void*)
#define IOCTL_IS_MODULE_RESET _IOR(9999, 'f', uint8_t*)

#define TRIM_USER_INPUT_ENABLED 0b00000001
#define OUTPUT_PREFIX_ENABLED 0b00000010
#define DEFAULT_SETTINGS 0b00000001

long ioctl_trim_user_input(const uint8_t* should_trim, uint8_t* module_settings);
long ioctl_get_chars_count_from_buffer(const char* module_buffer, size_t* buffer_size);
long ioctl_set_output_prefix(const void* output_prefix_data, char* module_output_prefix, uint8_t* module_settings);
long ioctl_get_output_prefix_size(const char* module_output_prefix, size_t* output_prefix_size);
long ioctl_do_module_reset(char* module_buffer, char* module_output_prefix, uint8_t* module_settings);
long ioctl_is_module_reset(const char* module_buffer, const uint8_t* module_settings, uint8_t* is_module_reset);

void reset_buffers(char* module_buffer, char* module_output_prefix);
