#pragma once

#include <linux/kobject.h>

#define SUCCESS 0
#define MAX_COMMAND_STR_LENGTH 32
#define MAX_STATUS_STR_LENGTH 16
#define MAX_KEY_STR_LENGTH 64
#define MAX_ELEMENTS_COUNT 256

static const char* update_command = "update";
static const char* remove_command = "remove";
static const char* get_command = "get";
static const char* reset_command = "reset";

static const size_t update_command_length = 6;
static const size_t remove_command_length = 6;
static const size_t get_command_length = 3;
static const size_t reset_command_length = 5;

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

void update_element(struct mapping_data* data, struct map_element_data** map_elements, size_t* current_elements_count,
                    struct map_element_data* (*create_element)(const char*, int));

void remove_element(struct mapping_data* data, struct map_element_data** map_elements, size_t* current_elements_count,
                    void (*destroy_element)(struct map_element_data* element_data));

void get_element_value(struct mapping_data* data, struct map_element_data** map_elements,
                       size_t current_elements_count);

void reset_elements(struct mapping_data* data, struct map_element_data** map_elements, size_t* current_elements_count,
                    void (*destroy_element)(struct map_element_data* element_data));
