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
    struct map_element_data* map_elements[MAX_ELEMENTS_COUNT];
    size_t map_elements_count;
};

struct map_element_data
{
    struct kobject map_element_kobj;
    int value;
};

int init_data(struct mapping_data* map_data, struct map_element_data* (*create_element)(const char*, int),
              void (*destroy_element)(struct map_element_data* element_data));

void clear_map_elements(void);

void store_key(const char* key_str);
int store_value(const char* value_str);
void store_command(const char* command_str);
