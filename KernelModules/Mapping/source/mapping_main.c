#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/sysfs.h>

#define SUCCESS 0
#define MAX_COMMAND_STR_LENGTH 32
#define MAX_STATUS_STR_LENGTH 16
#define MAX_KEY_STR_LENGTH 64

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This module illustrates a dictionary.");
MODULE_AUTHOR("Liviu Popa");

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
struct map_element_data* element_data_1 = NULL;
struct map_element_data* element_data_2 = NULL;

/* SYSFS access methods for attributes */

static ssize_t key_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
    struct mapping_data* data = container_of(kobj, struct mapping_data, mapping_kobj);
    return sysfs_emit(buf, "%s\n", data->key);
}

static ssize_t key_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    struct mapping_data* data = container_of(kobj, struct mapping_data, mapping_kobj);
    memset(data->key, '\0', MAX_KEY_STR_LENGTH);
    strncpy(data->key, buf, MAX_KEY_STR_LENGTH - 1);

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
    return result < 0 ? 0 : count;
}

// no show to be defined here as the command is write-only
static ssize_t command_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    struct mapping_data* data = container_of(kobj, struct mapping_data, mapping_kobj);

    // store_command(data, buf);
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
            strncpy(data->status, "synced", 6);
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
            if ((element_data_1 = create_map_element("First", -5)) == NULL)
            {
                kset_unregister(map_elements_kset);
                result = -EINVAL;
                break;
            }

            if ((element_data_2 = create_map_element("Second", 3)) == NULL)
            {
                kobject_put(&element_data_1->map_element_kobj);
                kfree(element_data_1);
                kset_unregister(map_elements_kset);
                result = -EINVAL;
            }
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
    kobject_put(&element_data_1->map_element_kobj);
    kobject_put(&element_data_2->map_element_kobj);
    kset_unregister(map_elements_kset);

    pr_info("%s: the module exited!\n", THIS_MODULE->name);
}

module_init(mapping_init);
module_exit(mapping_exit);
