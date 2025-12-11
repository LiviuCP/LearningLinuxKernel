#pragma once

#include <linux/kobject.h>

#define MAX_COMMAND_STR_LENGTH 32
#define MAX_STATUS_STR_LENGTH 16

struct division_data
{
    struct kobject division_kobj;
    int divided;
    int divider;
    int quotient;
    int remainder;
    char command[MAX_COMMAND_STR_LENGTH];
    char status[MAX_STATUS_STR_LENGTH];
};

int init_data(struct division_data* data, int divided, int divider);

int store_divided_value(struct division_data* data, const char* divided_str);
int store_divider_value(struct division_data* data, const char* divider_str);
void store_command(struct division_data* data, const char* command_str);
