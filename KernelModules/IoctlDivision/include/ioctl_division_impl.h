#pragma once

#define SUCCESS 0
#define SUPPORTED_MINOR_NUMBERS_COUNT 1

// 9999 is an arbitrarily chosen "magic number" (in a "real" (production) system an official assignment would be
// required; might be the major driver number)
#define IOCTL_STORE_DIVIDED_VALUE _IOW(9999, 'a', int*)
#define IOCTL_SHOW_DIVIDED_VALUE _IOR(9999, 'b', int*)
#define IOCTL_STORE_DIVIDER_VALUE _IOW(9999, 'c', int*)
#define IOCTL_SHOW_DIVIDER_VALUE _IOR(9999, 'd', int*)
#define IOCTL_SHOW_QUOTIENT_VALUE _IOR(9999, 'e', int*)
#define IOCTL_SHOW_REMAINDER_VALUE _IOR(9999, 'f', int*)
#define IOCTL_DIVIDE _IOW(9999, 'g', void*)
#define IOCTL_RESET _IOW(9999, 'h', void*)
#define IOCTL_GET_SYNCED_STATUS _IOR(9999, 'i', uint8_t*)

long ioctl_store_divided_value(const int* divided_value);
long ioctl_show_divided_value(int* divided_value);
long ioctl_store_divider_value(const int* divider_value);
long ioctl_show_divider_value(int* divider_value);
long ioctl_show_quotient_value(int* quotient_value);
long ioctl_show_remainder_value(int* remainder_value);
long ioctl_divide(void);
long ioctl_reset(void);
long ioctl_get_synced_status(uint8_t* is_synced_value);
