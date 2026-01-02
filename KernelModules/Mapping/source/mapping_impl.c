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
