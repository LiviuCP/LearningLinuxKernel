#pragma once

#include <linux/kobject.h>

#define COMMAND_BYTES_COUNT 32
#define STATUS_BYTES_COUNT 16

struct division_data
{
    struct kobject division_kobj;
    int divided;
    int divider;
    int quotient;
    int remainder;
    char command[COMMAND_BYTES_COUNT];
    char status[STATUS_BYTES_COUNT];
};

int init_data(struct division_data* data, int divided, int divider);

int store_divided_value(struct division_data* data, const char* divided_str);
int store_divider_value(struct division_data* data, const char* divider_str);
void store_command(struct division_data* data, const char* command_str);
