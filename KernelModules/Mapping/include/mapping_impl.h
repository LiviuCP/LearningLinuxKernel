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

int init_data(struct mapping_data* data);

void store_key(struct mapping_data* data, const char* key_str);
int store_value(struct mapping_data* data, const char* value_str);

void update_key_and_value(struct mapping_data* data, struct map_element_data** map_elements,
                          size_t* current_elements_count, struct map_element_data* (*create_element)(const char*, int));

void remove_key_and_value(struct mapping_data* data, struct map_element_data** map_elements,
                          size_t* current_elements_count,
                          void (*destroy_element)(struct map_element_data* element_data));

void reset_keys_and_values(struct mapping_data* data, struct map_element_data** map_elements,
                           size_t* current_elements_count,
                           void (*destroy_element)(struct map_element_data* element_data));

void get_value(struct mapping_data* data, struct map_element_data** map_elements, size_t current_elements_count);
