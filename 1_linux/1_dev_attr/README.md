1. sysfs_create_file
功能概述
sysfs_create_file 是一个底层函数，用于在指定的 kobject 对应的 sysfs 目录下创建一个属性文件。kobject 是 Linux 内核中用于表示对象的通用结构体，许多内核对象（如设备、驱动等）都是基于 kobject 实现的。该函数直接与 sysfs 的底层机制交互，为 kobject 关联的属性创建对应的文件。
函数原型
int sysfs_create_file(struct kobject *kobj, const struct attribute *attr);
参数说明：
kobj：指向 struct kobject 结构体的指针，代表要操作的内核对象。
attr：指向 struct attribute 结构体的指针，该结构体定义了要创建的属性文件的基本信息，如文件名、访问权限等。
使用场景
当你需要为一个通用的 kobject 类型对象创建单个属性文件时，可以使用该函数。比如，在自定义内核模块中创建一个简单的属性文件来暴露模块的某个状态。

2. device_create_file
功能概述
device_create_file 是一个相对高层的函数，专门用于在与设备相关的 sysfs 目录下创建属性文件。它简化了设备驱动开发者的工作，使得他们可以更方便地将设备的特定属性暴露给用户空间。
函数原型
int device_create_file(struct device *dev, const struct device_attribute *attr);
参数说明：
dev：指向 struct device 结构体的指针，代表要操作的设备。
attr：指向 struct device_attribute 结构体的指针，该结构体除了包含属性文件的基本信息外，还关联了用于读写该属性的回调函数。
使用场景
在设备驱动开发中，当你需要为某个设备创建单个属性文件时，使用该函数可以避免直接操作 kobject 带来的复杂性。

3. device_add_group
功能概述
device_add_group 用于一次性为设备添加一组属性文件。属性组由 struct attribute_group 结构体表示，该结构体可以包含多个属性，这样可以更方便地组织和管理设备的属性。
函数原型
int device_add_group(struct device *dev, const struct attribute_group *grp);
参数说明：
dev：指向 struct device 结构体的指针，代表要操作的设备。
grp：指向 struct attribute_group 结构体的指针，该结构体包含了一组属性的信息以及可选的操作函数。
使用场景
当设备有多个相关属性需要一起添加到 sysfs 中时，使用 device_add_group 可以提高代码的可读性和可维护性。

4. device_add_groups
功能
device_add_groups 可一次性为设备添加多个属性组，进一步增强了批量添加属性的能力，能更高效地组织大量设备属性。

区别总结
使用层次：
sysfs_create_file 是底层函数，直接操作 kobject，通用性强但使用复杂。
device_create_file 基于设备进行高层封装，为设备驱动开发者简化操作。
device_add_group 和 device_add_groups 进一步封装，用于批量添加属性，device_add_groups 能处理更多属性分组。
操作对象：
sysfs_create_file 操作 kobject，适用于各类内核对象。
device_create_file、device_add_group 和 device_add_groups 操作 struct device，用于设备相关操作。
创建粒度：
sysfs_create_file 和 device_create_file 每次创建一个属性文件。
device_add_group 一次性添加一组属性文件。
device_add_groups 一次性添加多个属性组。


