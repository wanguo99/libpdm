#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

static int __init test_init(void) {
    printk(KERN_INFO "Module loaded.\n");
    return 0;
}

static void __exit test_exit(void) {
    printk(KERN_INFO "Module unloaded.\n");
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wanguo");
MODULE_DESCRIPTION("A simple module.");
