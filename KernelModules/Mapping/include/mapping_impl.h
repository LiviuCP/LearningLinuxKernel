#pragma once

#include <linux/kobject.h>

#define SUCCESS 0
#define COMMAND_BYTES_COUNT 32
#define STATUS_BYTES_COUNT 16
#define KEY_BYTES_COUNT 64
#define MAX_ELEMENTS_COUNT 256

struct mapping_data
{
    struct kobject mapping_kobj;
    char key[KEY_BYTES_COUNT];
    int value;
    char command[COMMAND_BYTES_COUNT];
    char status[STATUS_BYTES_COUNT];
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
