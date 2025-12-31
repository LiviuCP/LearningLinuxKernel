#include <linux/ctype.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/sysfs.h>

#define SUCCESS 0
#define MAX_COMMAND_STR_LENGTH 32
#define MAX_STATUS_STR_LENGTH 16
#define MAX_KEY_STR_LENGTH 64
#define MAX_ELEMENTS_COUNT 256

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This module illustrates a dictionary.");
MODULE_AUTHOR("Liviu Popa");

static const char* dirty_status_str = "dirty";
static const char* synced_status_str = "synced";

/* VARIABLES AND PARAMETERS */

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

struct mapping_data* data = NULL;

static struct kset* map_elements_kset = NULL;
static struct map_element_data* map_elements[MAX_ELEMENTS_COUNT];
static size_t current_elements_count = 0;

static void trim_and_copy_str(char* dest, const char* src, size_t max_str_length)
{
    do
    {
        if (!dest)
        {
            pr_warn("%s: NULL dest string!\n", THIS_MODULE->name);
            break;
        }

        if (!src)
        {
            pr_warn("%s: NULL src string!\n", THIS_MODULE->name);
            break;
        }

        memset(dest, '\0', max_str_length);

        char temp[max_str_length];

        memset(temp, '\0', max_str_length);
        strncpy(temp, src, max_str_length - 1);

        const size_t temp_length = strlen(temp);

        if (temp_length == 0)
        {
            break;
        }

        size_t left_index = 0;
        size_t right_index = temp_length - 1;

        while (left_index <= right_index && isspace(temp[left_index]))
        {
            ++left_index;
        }

        while (left_index < right_index && isspace(temp[right_index]))
        {
            --right_index;
        }

        const size_t length = right_index >= left_index ? right_index - left_index + 1 : 0;
        const char* start = temp + left_index;

        strncpy(dest, start, length);
    } while (false);
}

/* SYSFS access methods for attributes */

static ssize_t key_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    struct mapping_data* data = container_of(kobj, struct mapping_data, mapping_kobj);
    return sysfs_emit(buf, "%s\n", data->key);
}

static ssize_t key_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    struct mapping_data* data = container_of(kobj, struct mapping_data, mapping_kobj);
    trim_and_copy_str(data->key, buf, MAX_KEY_STR_LENGTH);
    memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
    strncpy(data->status, dirty_status_str, strlen(dirty_status_str));
    pr_info("%s: new key entered: %s\n", THIS_MODULE->name, data->key);

    return count;
}

static ssize_t value_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    struct mapping_data* data = container_of(kobj, struct mapping_data, mapping_kobj);
    return sysfs_emit(buf, "%d\n", data->value);
}

static ssize_t value_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    struct mapping_data* data = container_of(kobj, struct mapping_data, mapping_kobj);
    const int result = kstrtoint(buf, 10, &data->value);

    if (result >= 0)
    {
        memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
        strncpy(data->status, dirty_status_str, strlen(dirty_status_str));
        pr_info("%s: new value entered: %d\n", THIS_MODULE->name, data->value);
    }

    return result < 0 ? 0 : count;
}

static struct map_element_data* create_map_element(const char* key, int value);

static void update_key_and_value(void)
{
    int should_add_element = 1;

    for (size_t index = 0; index < current_elements_count; ++index)
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
        if (current_elements_count < MAX_ELEMENTS_COUNT)
        {
            struct map_element_data* element_data = create_map_element(data->key, data->value);

            if (element_data)
            {
                map_elements[current_elements_count] = element_data;
                ++current_elements_count;
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

static void destroy_map_element(struct map_element_data* element_data)
{
    if (element_data)
    {
        kobject_put(&element_data->map_element_kobj);
    }
}

static void remove_key_and_value(void)
{
    int element_found = 0;

    for (size_t index = 0; index < current_elements_count; ++index)
    {
        if (strncmp(map_elements[index]->map_element_kobj.name, data->key, strlen(data->key)) == 0)
        {
            element_found = 1;
            destroy_map_element(map_elements[index]);

            if (current_elements_count > 1 && index < current_elements_count - 1)
            {
                map_elements[index] = map_elements[current_elements_count - 1];
            }

            map_elements[current_elements_count - 1] = NULL;
            --current_elements_count;
            break;
        }
    }

    if (element_found)
    {
        memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
        strncpy(data->status, synced_status_str, strlen(synced_status_str));
    }
    else
    {
        pr_err("%s: cannot remove element, key %s has not been found\n", THIS_MODULE->name, data->key);
    }
}

static void reset_keys_and_values(void)
{
    for (size_t index = 0; index < current_elements_count; ++index)
    {
        destroy_map_element(map_elements[index]);
        map_elements[index] = NULL;
    }

    current_elements_count = 0;
    memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
    strncpy(data->status, synced_status_str, strlen(synced_status_str));
}

static void get_value(void)
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

// no show to be defined here as the command is write-only (TODO: update the command checking mechanism - see Division)
static ssize_t command_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    struct mapping_data* data = container_of(kobj, struct mapping_data, mapping_kobj);

    trim_and_copy_str(data->command, buf, MAX_COMMAND_STR_LENGTH);
    const size_t command_length = strlen(data->command);

    pr_info("%s: issued command: %s\n", THIS_MODULE->name, data->command);

    if (command_length == 6 && strncmp(data->command, "update", 6) == 0)
    {
        pr_info("%s: updating key/value\n", THIS_MODULE->name);
        update_key_and_value();
    }
    else if (command_length == 6 && strncmp(data->command, "remove", 6) == 0)
    {
        pr_info("%s: removing key/value\n", THIS_MODULE->name);
        remove_key_and_value();
    }
    else if (command_length == 3 && strncmp(data->command, "get", 3) == 0)
    {
        pr_info("%s: getting value from key\n", THIS_MODULE->name);
        get_value();
    }
    else if (command_length == 5 && strncmp(data->command, "reset", 5) == 0)
    {
        pr_info("%s: resetting all map elements\n", THIS_MODULE->name);
        reset_keys_and_values();
    }
    else
    {
        pr_warn("%s: invalid command: %s\n", THIS_MODULE->name, data->command);
    }

    return count;
}

// no store to be defined here as the status is read-only
static ssize_t status_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    struct mapping_data* data = container_of(kobj, struct mapping_data, mapping_kobj);
    return sysfs_emit(buf, "%s\n", data->status);
}

static void mapping_release(struct kobject* kobj)
{
    struct mapping_data* data = container_of(kobj, struct mapping_data, mapping_kobj);
    pr_info("%s: freeing data object that contains kobject \"%s\"\n", THIS_MODULE->name, kobj->name);
    kfree(data);
}

static void map_element_release(struct kobject* kobj)
{
    struct map_element_data* data = container_of(kobj, struct map_element_data, map_element_kobj);
    pr_info("%s: freeing map element data object that contains kobject \"%s\"\n", THIS_MODULE->name, kobj->name);
    kfree(data);
}

/* SYSFS attributes for Mapping */

static struct kobj_attribute key_attribute = __ATTR(key, 0600, key_show, key_store);
static struct kobj_attribute value_attribute = __ATTR(value, 0600, value_show, value_store);
static struct kobj_attribute command_attribute = __ATTR(command, 0200, NULL, command_store);
static struct kobj_attribute status_attribute = __ATTR(status, 0400, status_show, NULL);

static struct attribute* mapping_attrs[] = {&key_attribute.attr, &value_attribute.attr, &command_attribute.attr,
                                            &status_attribute.attr, NULL};

// these statements can be replaced by: "ATTRIBUTE_GROUPS(mapping);" (see sysfs.h)
// I preferred to keep the unwrapped for better readability
static const struct attribute_group mapping_group = {.attrs = mapping_attrs};
static const struct attribute_group* mapping_groups[] = {&mapping_group, NULL};

// kobj_sysfs_ops is the default sysfs_ops (binds all access functions defined above into mapping_ktype)
static struct kobj_type mapping_ktype = {
    .sysfs_ops = &kobj_sysfs_ops, .release = mapping_release, .default_groups = mapping_groups};

/* SYSFS attributes for MapElement */

static ssize_t element_value_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    struct map_element_data* data = container_of(kobj, struct map_element_data, map_element_kobj);
    return sysfs_emit(buf, "%d\n", data->value);
}

static struct kobj_attribute element_value_attribute = __ATTR(value, 0400, element_value_show, NULL);
static struct attribute* map_element_attrs[] = {&element_value_attribute.attr, NULL};

ATTRIBUTE_GROUPS(map_element);

static const struct kobj_type map_element_ktype = {
    .sysfs_ops = &kobj_sysfs_ops, .release = map_element_release, .default_groups = map_element_groups};

static struct map_element_data* create_map_element(const char* key, int value)
{
    struct map_element_data* data = NULL;

    if ((data = kzalloc(sizeof(struct map_element_data), GFP_KERNEL)) == NULL)
    {
        data = ERR_PTR(-ENOMEM);
    }

    if (data != ERR_PTR(-ENOMEM))
    {
        data->map_element_kobj.kset = map_elements_kset;

        const int result = kobject_init_and_add(&data->map_element_kobj, &map_element_ktype, NULL, "%s", key);

        if (!result)
        {
            data->value = value;
            kobject_uevent(&data->map_element_kobj, KOBJ_ADD);
        }
        else
        {
            kfree(data);
            data = ERR_PTR(-ENOMEM);
        }
    }

    return data;
}

/* INIT/EXIT */

static int mapping_init(void)
{
    for (size_t index = 0; index < MAX_ELEMENTS_COUNT; ++index)
    {
        map_elements[index] = NULL;
    }

    // resulting directory structure: "/sys/kernel/mapping"
    // - kernel_kobj (parent kobject) => "/sys/kernel"
    // - mapping_kobj_name => "/mapping"
    // - all attributes appearing as files in "/mapping"
    const char* mapping_kobj_name = "mapping";

    int result = SUCCESS;

    pr_info("%s: initializing module\n", THIS_MODULE->name);
    data = kzalloc(sizeof(struct mapping_data), GFP_KERNEL);

    if (data)
    {
        result = kobject_init_and_add(&data->mapping_kobj, &mapping_ktype, kernel_kobj, "%s", mapping_kobj_name);

        if (result == SUCCESS)
        {
            memset(data->key, '\0', MAX_KEY_STR_LENGTH);
            data->value = 0;
            memset(data->command, '\0', MAX_COMMAND_STR_LENGTH);
            memset(data->status, '\0', MAX_STATUS_STR_LENGTH);
            strncpy(data->status, synced_status_str, strlen(synced_status_str));
        }
        else
        {
            result = -ENOMEM;
        }
    }
    else
    {
        result = -ENOMEM;
    }

    if (result == SUCCESS)
    {
        kobject_uevent(&data->mapping_kobj, KOBJ_ADD);

        map_elements_kset = kset_create_and_add("Map", NULL, &data->mapping_kobj);

        do
        {
            if ((map_elements[0] = create_map_element("First", -5)) == NULL)
            {
                kset_unregister(map_elements_kset);
                result = -EINVAL;
                break;
            }

            ++current_elements_count;

            if ((map_elements[1] = create_map_element("Second", 3)) == NULL)
            {
                destroy_map_element(map_elements[0]);
                map_elements[0] = NULL;
                kset_unregister(map_elements_kset);
                result = -EINVAL;
                break;
            }

            ++current_elements_count;
        } while (false);
    }
    else if (result == -ENOMEM)
    {
        pr_err("%s: unable to initialize sysfs object \"%s\" (no memory)!\n", THIS_MODULE->name, mapping_kobj_name);
    }

    return result;
}

static void mapping_exit(void)
{
    pr_info("%s: putting the kobject: %s\n", THIS_MODULE->name, data->mapping_kobj.name);

    // this calls the release function (mapping_release()) for the data object containing the kobject
    kobject_put(&data->mapping_kobj);

    for (size_t index = 0; index < current_elements_count; ++index)
    {
        destroy_map_element(map_elements[index]);
        map_elements[index] = NULL;
    }

    kset_unregister(map_elements_kset);

    pr_info("%s: the module exited!\n", THIS_MODULE->name);
}

module_init(mapping_init);
module_exit(mapping_exit);
