#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sysfs.h>

#include "mapping_impl.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(
    "This module illustrates a dictionary (map). Each map element contains a key (directory) and a value (file).\n"
    "It is possible to add new elements, modify values of "
    "existing ones, retrieve element values based on keys, remove elements and erase the whole content.\n"
    "No specific optimizations are being used (e.g. the search time is linear, not logarithmic) as they are out of "
    "project scope."
    "The goal is to illustrate the kset concept and not to create an optimized dictionary.");
MODULE_AUTHOR("Liviu Popa");

/* VARIABLES AND PARAMETERS */

static struct mapping_data* data = NULL;
static struct kset* map_elements_kset = NULL;

/* SYSFS access functions for attributes */

static ssize_t key_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    struct mapping_data* data = container_of(kobj, struct mapping_data, mapping_kobj);
    return sysfs_emit(buf, "%s\n", data->key);
}

static ssize_t key_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    if (container_of(kobj, struct mapping_data, mapping_kobj) == data)
    {
        store_key(buf);
    }
    else
    {
        pr_err("%s: invalid mapping data object!\n", THIS_MODULE->name);
    }

    return count;
}

static ssize_t value_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    struct mapping_data* data = container_of(kobj, struct mapping_data, mapping_kobj);
    return sysfs_emit(buf, "%d\n", data->value);
}

static ssize_t value_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    int result = -1;

    if (container_of(kobj, struct mapping_data, mapping_kobj) == data)
    {
        result = store_value(buf);
    }
    else
    {
        pr_err("%s: invalid mapping data object!\n", THIS_MODULE->name);
    }

    return result < 0 ? 0 : count;
}

// no store to be defined here as the elements count is read-only
static ssize_t count_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    struct mapping_data* data = container_of(kobj, struct mapping_data, mapping_kobj);
    return sysfs_emit(buf, "%ld\n", data ? (ssize_t)data->map_elements_count : 0);
}

// no show to be defined here as the command is write-only
static ssize_t command_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    if (container_of(kobj, struct mapping_data, mapping_kobj) == data)
    {
        store_command(buf);
    }
    else
    {
        pr_err("%s: invalid mapping data object!\n", THIS_MODULE->name);
    }

    return count;
}

// no store to be defined here as the status is read-only
static ssize_t status_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    struct mapping_data* data = container_of(kobj, struct mapping_data, mapping_kobj);
    return sysfs_emit(buf, "%s\n", data->status);
}

/* SYSFS release functions */

static void mapping_release(struct kobject* kobj)
{
    struct mapping_data* data = container_of(kobj, struct mapping_data, mapping_kobj);
    pr_info("%s: freeing mapping data object that contains kobject \"%s\"\n", THIS_MODULE->name, kobj->name);
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
static struct kobj_attribute count_attribute = __ATTR(count, 0400, count_show, NULL);
static struct kobj_attribute command_attribute = __ATTR(command, 0200, NULL, command_store);
static struct kobj_attribute status_attribute = __ATTR(status, 0400, status_show, NULL);

static struct attribute* mapping_attrs[] = {&key_attribute.attr,     &value_attribute.attr,  &count_attribute.attr,
                                            &command_attribute.attr, &status_attribute.attr, NULL};

ATTRIBUTE_GROUPS(mapping);

static struct kobj_type mapping_ktype = {
    .sysfs_ops = &kobj_sysfs_ops, .release = mapping_release, .default_groups = mapping_groups};

/* SYSFS attributes for map element */

static ssize_t element_value_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    struct map_element_data* data = container_of(kobj, struct map_element_data, map_element_kobj);
    return sysfs_emit(buf, "%d\n", data->value);
}

static struct kobj_attribute element_value_attribute = __ATTR(value, 0400, element_value_show, NULL);
static struct attribute* map_element_attrs[] = {&element_value_attribute.attr, NULL};

ATTRIBUTE_GROUPS(map_element);

static struct kobj_type map_element_ktype = {
    .sysfs_ops = &kobj_sysfs_ops, .release = map_element_release, .default_groups = map_element_groups};

/* "CONSTRUCTOR"/"DESTRUCTOR" for map elements */

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

static void destroy_map_element(struct map_element_data* element_data)
{
    if (element_data)
    {
        pr_info("%s: putting the kobject for map element: \"%s\"\n", THIS_MODULE->name,
                element_data->map_element_kobj.name);
        kobject_put(&element_data->map_element_kobj);
    }
}

/* INIT/EXIT */

// resulting directory structure: "/sys/kernel/mapping"
// - kernel_kobj (parent kobject) => "/sys/kernel"
// - mapping_kobj_name => "/mapping"
// - all attributes appearing as files in "/mapping": key, value, count, command, status
// - map_elements_kset => "/mapping/Map"
// - map element kobjects (connected to kset) => "/mapping/Map/[KEY1]", "/mapping/Map/[KEY2]", ...
// - attribute of each map element kobject => "/mapping/Map/[KEY1]/value", "/mapping/Map/[KEY2]/value", ...
static int mapping_init(void)
{
    const char* mapping_kobj_name = "mapping";

    int result = SUCCESS;

    pr_info("%s: initializing module\n", THIS_MODULE->name);
    data = kzalloc(sizeof(struct mapping_data), GFP_KERNEL);

    if (data)
    {
        result = kobject_init_and_add(&data->mapping_kobj, &mapping_ktype, kernel_kobj, "%s", mapping_kobj_name);
        result = result == SUCCESS ? init_data(data, create_map_element, destroy_map_element) : -ENOMEM;
    }
    else
    {
        result = -ENOMEM;
    }

    if (result == SUCCESS)
    {
        kobject_uevent(&data->mapping_kobj, KOBJ_ADD);
        map_elements_kset = kset_create_and_add("Map", NULL, &data->mapping_kobj);
    }
    else if (result == -ENOMEM)
    {
        pr_err("%s: unable to initialize sysfs object \"%s\" (no memory)!\n", THIS_MODULE->name, mapping_kobj_name);
    }

    return result;
}

static void mapping_exit(void)
{
    clear_map_elements();

    pr_info("%s: putting the mapping kobject: \"%s\"\n", THIS_MODULE->name, data->mapping_kobj.name);

    kobject_put(&data->mapping_kobj);
    kset_unregister(map_elements_kset);

    pr_info("%s: the module exited!\n", THIS_MODULE->name);
}

module_init(mapping_init);
module_exit(mapping_exit);
