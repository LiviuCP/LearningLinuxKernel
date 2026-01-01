#pragma once

#include <linux/kobject.h>

#define SUCCESS 0
#define MAX_COMMAND_STR_LENGTH 32
#define MAX_STATUS_STR_LENGTH 16
#define MAX_KEY_STR_LENGTH 64
#define MAX_ELEMENTS_COUNT 256

struct mapping_data
{
    struct kobject mapping_kobj;
    char key[MAX_KEY_STR_LENGTH];
    int value;
    char command[MAX_COMMAND_STR_LENGTH];
    char status[MAX_STATUS_STR_LENGTH];
};

struct map_element_data
{
    struct kobject map_element_kobj;
    int value;
};
