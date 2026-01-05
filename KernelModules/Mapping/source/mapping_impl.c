#include <linux/module.h>

#include "mapping_impl.h"
#include "mapping_utils.h"

#define ENULLDATAOBJECT 2
#define EOTHERNULLOBJECT 3

MODULE_LICENSE("GPL");

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

static void update_map_element(void)
{
    do
    {
        if (!data || !create_map_element)
        {
            break;
        }

        const size_t key_length = strlen(data->key);

        if (key_length == 0)
        {
            pr_err("%s: cannot add element, key is empty\n", THIS_MODULE->name);
            break;
        }

        int should_add_element = 1;

        for (size_t index = 0; index < data->map_elements_count; ++index)
        {
            const char* kobj_name = data->map_elements[index]->map_element_kobj.name;

            if (strlen(kobj_name) != key_length || strncmp(kobj_name, data->key, key_length) != 0)
            {
                continue;
            }

            data->map_elements[index]->value = data->value;
            should_add_element = 0;
            pr_info("%s: updated element: (key: %s, value: %d)\n", THIS_MODULE->name, data->key, data->value);
            break;
        }

        if (!should_add_element)
        {
            memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
            strncpy(data->status, synced_status_str, strlen(synced_status_str));
            break;
        }

        if (data->map_elements_count == MAX_ELEMENTS_COUNT)
        {
            pr_err("%s: cannot add element, maximum count has been reached\n", THIS_MODULE->name);
            break;
        }

        struct map_element_data* element_data = create_map_element(data->key, data->value);

        if (!element_data)
        {
            pr_err("%s: could not create new element: (key: %s, value: %d)\n", THIS_MODULE->name, data->key,
                   data->value);
            break;
        }

        data->map_elements[data->map_elements_count] = element_data;
        ++data->map_elements_count;
        pr_info("%s: added element: (key: %s, value: %d)\n", THIS_MODULE->name, data->key, data->value);
        memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
        strncpy(data->status, synced_status_str, strlen(synced_status_str));
    } while (false);
}

static void remove_map_element(void)
{
    do
    {
        if (!data || !destroy_map_element)
        {
            break;
        }

        int element_found = 0;
        const size_t key_length = strlen(data->key);

        for (size_t index = 0; index < data->map_elements_count; ++index)
        {
            const char* kobj_name = data->map_elements[index]->map_element_kobj.name;

            if (strlen(kobj_name) != key_length || strncmp(kobj_name, data->key, key_length) != 0)
            {
                continue;
            }

            element_found = 1;
            destroy_map_element(data->map_elements[index]);

            if (data->map_elements_count > 1 && index < data->map_elements_count - 1)
            {
                data->map_elements[index] = data->map_elements[data->map_elements_count - 1];
            }

            data->map_elements[data->map_elements_count - 1] = NULL;
            --data->map_elements_count;
            pr_info("%s: removed element with key: %s\n", THIS_MODULE->name, data->key);
            break;
        }

        if (!element_found)
        {
            pr_err("%s: cannot remove element, key %s has not been found\n", THIS_MODULE->name, data->key);
            break;
        }

        memset(data->key, '\0', MAX_KEY_STR_LENGTH);
        data->value = 0;
        memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
        strncpy(data->status, synced_status_str, strlen(synced_status_str));
    } while (false);
}

static void retrieve_map_element_value(void)
{
    if (data)
    {
        int element_found = 0;
        const size_t key_length = strlen(data->key);

        for (size_t index = 0; index < data->map_elements_count; ++index)
        {
            const char* kobj_name = data->map_elements[index]->map_element_kobj.name;

            if (strlen(kobj_name) != key_length || strncmp(kobj_name, data->key, key_length) != 0)
            {
                continue;
            }

            element_found = 1;
            data->value = data->map_elements[index]->value;
            pr_info("%s: retrieved value %d for element with key %s\n", THIS_MODULE->name, data->value, data->key);
            break;
        }

        if (!element_found)
        {
            pr_warn("%s: key %s has not been found, setting default value\n", THIS_MODULE->name, data->key);
            data->value = 0;
        }

        memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
        strncpy(data->status, synced_status_str, strlen(synced_status_str));
    }
}

static void erase_map_elements(void)
{
    if (data && destroy_map_element)
    {
        for (size_t index = 0; index < data->map_elements_count; ++index)
        {
            destroy_map_element(data->map_elements[index]);
            data->map_elements[index] = NULL;
        }

        if (data->map_elements_count > 0)
        {
            pr_info("%s: erased all map elements\n", THIS_MODULE->name);
        }
        else
        {
            pr_warn("%s: no map elements to erase\n", THIS_MODULE->name);
        }

        data->map_elements_count = 0;
        memset(data->key, '\0', MAX_KEY_STR_LENGTH);
        data->value = 0;
        memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
        strncpy(data->status, synced_status_str, strlen(synced_status_str));
    }
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

        data->map_elements_count = 0;

        for (size_t index = 0; index < MAX_ELEMENTS_COUNT; ++index)
        {
            data->map_elements[index] = NULL;
        }
    }
    else
    {
        result = !map_data ? -ENULLDATAOBJECT : -EOTHERNULLOBJECT;
        pr_warn("%s: NULL data or function object!\n", THIS_MODULE->name);
    }

    return result;
}

void clear_map_elements(void)
{
    if (data && destroy_map_element)
    {
        for (size_t index = 0; index < data->map_elements_count; ++index)
        {
            destroy_map_element(data->map_elements[index]);
            data->map_elements[index] = NULL;
        }
    }
    else
    {
        pr_err("%s: NULL data or function object (possibly not correctly initialized)\n", THIS_MODULE->name);
    }
}

void store_key(const char* key_str)
{
    if (data)
    {
        trim_and_copy_str(data->key, key_str, MAX_KEY_STR_LENGTH);
        memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
        strncpy(data->status, dirty_status_str, strlen(dirty_status_str));
        pr_info("%s: key entered: %s\n", THIS_MODULE->name, data->key);
    }
    else
    {
        pr_err("%s: NULL data object (possibly not correctly initialized)\n", THIS_MODULE->name);
    }
}

int store_value(const char* value_str)
{
    const int result = data ? kstrtoint(value_str, 10, &data->value) : -ENULLDATAOBJECT;

    if (result >= 0)
    {
        memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
        strncpy(data->status, dirty_status_str, strlen(dirty_status_str));
        pr_info("%s: value entered: %d\n", THIS_MODULE->name, data->value);
    }
    else if (result == -ENULLDATAOBJECT)
    {
        pr_err("%s: NULL data object (possibly not correctly initialized)\n", THIS_MODULE->name);
    }

    return result;
}

void store_command(const char* command_str)
{
    if (data && create_map_element && destroy_map_element)
    {
        trim_and_copy_str(data->command, command_str, MAX_COMMAND_STR_LENGTH);
        const size_t command_length = strlen(data->command);

        pr_info("%s: issued command: %s\n", THIS_MODULE->name, data->command);

        if (command_length == update_command_length &&
            strncmp(data->command, update_command, update_command_length) == 0)
        {
            update_map_element();
        }
        else if (command_length == remove_command_length &&
                 strncmp(data->command, remove_command, remove_command_length) == 0)
        {
            remove_map_element();
        }
        else if (command_length == get_command_length && strncmp(data->command, get_command, get_command_length) == 0)
        {
            retrieve_map_element_value();
        }
        else if (command_length == reset_command_length &&
                 strncmp(data->command, reset_command, reset_command_length) == 0)
        {
            erase_map_elements();
        }
        else
        {
            pr_warn("%s: invalid command: %s\n", THIS_MODULE->name, data->command);
        }
    }
    else
    {
        pr_err("%s: NULL data or function object (possibly not correctly initialized)\n", THIS_MODULE->name);
    }
}
