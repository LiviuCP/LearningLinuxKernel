#pragma once

long ioctl_store_divided_value(const int* divided_value);
long ioctl_show_divided_value(int* divided_value);
long ioctl_store_divider_value(const int* divider_value);
long ioctl_show_divider_value(int* divider_value);
long ioctl_show_quotient_value(int* quotient_value);
long ioctl_show_remainder_value(int* remainder_value);
long ioctl_divide(void);
long ioctl_reset(void);
long ioctl_get_synced_status(bool* is_synced_value);
