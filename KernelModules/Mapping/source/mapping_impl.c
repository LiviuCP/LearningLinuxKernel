#include <linux/module.h>

#include "mapping_impl.h"
#include "mapping_utils.h"

#define ENULLDATAOBJECT 2

static const char* dirty_status_str = "dirty";
static const char* synced_status_str = "synced";

static const char* update_command = "update";
static const char* remove_command = "remove";
static const char* get_command = "get";
static const char* reset_command = "reset";

static const size_t update_command_length = 6;
static const size_t remove_command_length = 6;
static const size_t get_command_length = 3;
static const size_t reset_command_length = 5;

static struct map_element_data* (*create_map_element)(const char*, int) = NULL;
static void (*destroy_map_element)(struct map_element_data* element_data) = NULL;

static struct mapping_data* data = NULL;

static void update_element(void)
{
    if (strlen(data->key) > 0)
    {
        int should_add_element = 1;

        for (size_t index = 0; index < data->current_elements_count; ++index)
        {
            if (strncmp(data->map_elements[index]->map_element_kobj.name, data->key, strlen(data->key)) == 0)
            {
                data->map_elements[index]->value = data->value;
                should_add_element = 0;
                break;
            }
        }

        if (should_add_element)
        {
            if (data->current_elements_count < MAX_ELEMENTS_COUNT)
            {
                struct map_element_data* element_data = create_map_element(data->key, data->value);

                if (element_data)
                {
                    data->map_elements[data->current_elements_count] = element_data;
                    ++data->current_elements_count;
                }
            }
            else
            {
                pr_err("%s: cannot add element, maximum count has been reached\n", THIS_MODULE->name);
            }
        }

        memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
        strncpy(data->status, synced_status_str, strlen(synced_status_str));
    }
    else
    {
        pr_warn("%s: cannot add element, key is empty\n", THIS_MODULE->name);
    }
}

static void remove_element(void)
{
    int element_found = 0;

    for (size_t index = 0; index < data->current_elements_count; ++index)
    {
        if (strncmp(data->map_elements[index]->map_element_kobj.name, data->key, strlen(data->key)) == 0)
        {
            element_found = 1;
            destroy_map_element(data->map_elements[index]);

            if (data->current_elements_count > 1 && index < data->current_elements_count - 1)
            {
                data->map_elements[index] = data->map_elements[data->current_elements_count - 1];
            }

            data->map_elements[data->current_elements_count - 1] = NULL;
            --data->current_elements_count;
            break;
        }
    }

    if (element_found)
    {
        memset(data->key, '\0', MAX_KEY_STR_LENGTH);
        data->value = 0;
        memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
        strncpy(data->status, synced_status_str, strlen(synced_status_str));
    }
    else
    {
        pr_err("%s: cannot remove element, key %s has not been found\n", THIS_MODULE->name, data->key);
    }
}

static void get_element_value(void)
{
    int element_found = 0;

    for (size_t index = 0; index < data->current_elements_count; ++index)
    {
        if (strncmp(data->map_elements[index]->map_element_kobj.name, data->key, strlen(data->key)) == 0)
        {
            element_found = 1;
            data->value = data->map_elements[index]->value;
            break;
        }
    }

    if (!element_found)
    {
        pr_warn("%s: key %s has not been found, setting default value\n", THIS_MODULE->name, data->key);
        data->value = 0;
    }

    memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
    strncpy(data->status, synced_status_str, strlen(synced_status_str));
}

static void reset_elements(void)
{
    for (size_t index = 0; index < data->current_elements_count; ++index)
    {
        destroy_map_element(data->map_elements[index]);
        data->map_elements[index] = NULL;
    }

    data->current_elements_count = 0;
    memset(data->key, '\0', MAX_KEY_STR_LENGTH);
    data->value = 0;
    memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
    strncpy(data->status, synced_status_str, strlen(synced_status_str));
}

int init_data(struct mapping_data* map_data, struct map_element_data* (*create_element)(const char*, int),
              void (*destroy_element)(struct map_element_data* element_data))
{
    int result = 0;

    if (map_data && create_element && destroy_element)
    {
        data = map_data;
        memset(data->key, '\0', MAX_KEY_STR_LENGTH);
        data->value = 0;
        memset(data->command, '\0', MAX_COMMAND_STR_LENGTH);
        memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
        strncpy(data->status, synced_status_str, strlen(synced_status_str));
        create_map_element = create_element;
        destroy_map_element = destroy_element;

        data->current_elements_count = 0;

        for (size_t index = 0; index < MAX_ELEMENTS_COUNT; ++index)
        {
            data->map_elements[index] = NULL;
        }
    }
    else
    {
        result = -ENULLDATAOBJECT;
        pr_warn("%s: NULL data object!\n", THIS_MODULE->name);
    }

    return result;
}

void store_key(const char* key_str)
{
    trim_and_copy_str(data->key, key_str, MAX_KEY_STR_LENGTH);
    memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
    strncpy(data->status, dirty_status_str, strlen(dirty_status_str));
    pr_info("%s: new key entered: %s\n", THIS_MODULE->name, data->key);
}

int store_value(const char* value_str)
{
    const int result = kstrtoint(value_str, 10, &data->value);

    if (result >= 0)
    {
        memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
        strncpy(data->status, dirty_status_str, strlen(dirty_status_str));
        pr_info("%s: new value entered: %d\n", THIS_MODULE->name, data->value);
    }

    return result;
}

void store_command(const char* command_str)
{
    trim_and_copy_str(data->command, command_str, MAX_COMMAND_STR_LENGTH);
    const size_t command_length = strlen(data->command);

    pr_info("%s: issued command: %s\n", THIS_MODULE->name, data->command);

    if (command_length == update_command_length && strncmp(data->command, update_command, update_command_length) == 0)
    {
        pr_info("%s: updating map element\n", THIS_MODULE->name);
        update_element();
    }
    else if (command_length == remove_command_length &&
             strncmp(data->command, remove_command, remove_command_length) == 0)
    {
        pr_info("%s: removing map element\n", THIS_MODULE->name);
        remove_element();
    }
    else if (command_length == get_command_length && strncmp(data->command, get_command, get_command_length) == 0)
    {
        pr_info("%s: getting value of map element\n", THIS_MODULE->name);
        get_element_value();
    }
    else if (command_length == reset_command_length && strncmp(data->command, reset_command, reset_command_length) == 0)
    {
        pr_info("%s: erasing all map elements\n", THIS_MODULE->name);
        reset_elements();
    }
    else
    {
        pr_warn("%s: invalid command: %s\n", THIS_MODULE->name, data->command);
    }
}
