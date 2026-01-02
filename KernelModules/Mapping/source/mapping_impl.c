#include <linux/module.h>

#include "mapping_impl.h"

void update_key_and_value(struct mapping_data* data, struct map_element_data** map_elements,
                          size_t* current_elements_count, struct map_element_data* (*create_element)(const char*, int))
{
    if (strlen(data->key) > 0)
    {
        int should_add_element = 1;

        for (size_t index = 0; index < *current_elements_count; ++index)
        {
            if (strncmp(map_elements[index]->map_element_kobj.name, data->key, strlen(data->key)) == 0)
            {
                map_elements[index]->value = data->value;
                should_add_element = 0;
                break;
            }
        }

        if (should_add_element)
        {
            if (*current_elements_count < MAX_ELEMENTS_COUNT)
            {
                struct map_element_data* element_data = create_element(data->key, data->value);

                if (element_data)
                {
                    map_elements[*current_elements_count] = element_data;
                    ++*current_elements_count;
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

void remove_key_and_value(struct mapping_data* data, struct map_element_data** map_elements,
                          size_t* current_elements_count,
                          void (*destroy_element)(struct map_element_data* element_data))
{
    int element_found = 0;

    for (size_t index = 0; index < *current_elements_count; ++index)
    {
        if (strncmp(map_elements[index]->map_element_kobj.name, data->key, strlen(data->key)) == 0)
        {
            element_found = 1;
            destroy_element(map_elements[index]);

            if (*current_elements_count > 1 && index < *current_elements_count - 1)
            {
                map_elements[index] = map_elements[*current_elements_count - 1];
            }

            map_elements[*current_elements_count - 1] = NULL;
            --*current_elements_count;
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

void reset_keys_and_values(struct mapping_data* data, struct map_element_data** map_elements,
                           size_t* current_elements_count,
                           void (*destroy_element)(struct map_element_data* element_data))
{
    for (size_t index = 0; index < *current_elements_count; ++index)
    {
        destroy_element(map_elements[index]);
        map_elements[index] = NULL;
    }

    *current_elements_count = 0;
    memset(data->key, '\0', MAX_KEY_STR_LENGTH);
    data->value = 0;
    memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
    strncpy(data->status, synced_status_str, strlen(synced_status_str));
}

void get_value(struct mapping_data* data, struct map_element_data** map_elements, size_t current_elements_count)
{
    int element_found = 0;

    for (size_t index = 0; index < current_elements_count; ++index)
    {
        if (strncmp(map_elements[index]->map_element_kobj.name, data->key, strlen(data->key)) == 0)
        {
            element_found = 1;
            data->value = map_elements[index]->value;
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
