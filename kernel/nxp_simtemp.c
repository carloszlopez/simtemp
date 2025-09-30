/************************************
 * INCLUDES
 ************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include "nxp_simtemp.h"

/************************************
 * STATIC FUNCTION PROTOTYPES
 ************************************/
#if LOCAL_DEVICE
static void nxp_simtemp_release(struct device *device);
#endif
static int nxp_simtemp_init(void);
static void nxp_simtemp_exit(void);
static int nxp_simtemp_probe(struct platform_device *drvrptr);
static void nxp_simtemp_remove(struct platform_device *drvrptr);
static ssize_t nxp_simtemp_read(struct file *filep, char __user *buf, 
                                size_t count, loff_t *offset);

/************************************
 * STATIC VARIABLES
 ************************************/
#if LOCAL_DEVICE
static struct platform_device nxp_simtemp_device = {
    .name = "nxp_simtemp",
    .id = -1,
    .dev = {
        .release = nxp_simtemp_release,
    },
};
#endif

static struct platform_driver nxp_simtemp_driver = {
    .probe = nxp_simtemp_probe,
    .remove = nxp_simtemp_remove,
    .driver = {
        .name = "nxp_simtemp",
        .owner = THIS_MODULE,
    },
};

static const struct file_operations nxp_simtemp_fops = {
    .owner = THIS_MODULE,
    .read  = nxp_simtemp_read,
};

static struct miscdevice nxp_simtemp_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "nxp_simtemp",
    .fops = &nxp_simtemp_fops,
};
/************************************
 * STATIC FUNCTIONS
 ************************************/
static int nxp_simtemp_init(void) {
    int ret;
    
    printk("nxp_simtemp: init\n");
    /* Register driver */
    ret = platform_driver_register(&nxp_simtemp_driver);
    if ( ret) {
        printk("nxp_simtemp: driver register error\n");
    } else {
#if LOCAL_DEVICE
        /* Register device */
        ret = platform_device_register(&nxp_simtemp_device);
        if (ret) {
            printk("nxp_simtemp: device register error\n");
        }
        else {
            /* Do nothing */
        }
#endif
    }
    return ret;
}

static void nxp_simtemp_exit(void) {
    printk("nxp_simtemp: exit\n");
    /* Unregister driver and device */
    platform_driver_unregister(&nxp_simtemp_driver);
    platform_device_unregister(&nxp_simtemp_device);
}

#if LOCAL_DEVICE
static void nxp_simtemp_release(struct device *device) {
    printk("nxp_simtemp: release\n");
}
#endif

static int nxp_simtemp_probe(struct platform_device *device) {
    printk("nxp_simtemp: release\n");
    /* Register character device */
    return misc_register(&nxp_simtemp_miscdev);
}

static void nxp_simtemp_remove(struct platform_device *device) {
    printk("nxp_simtemp: remove\n");
    /* Deregister character device */
    return misc_deregister(&nxp_simtemp_miscdev);
}

static ssize_t nxp_simtemp_read(struct file *filep, char __user *buf, 
                                size_t count, loff_t *offset) {
    printk("nxp_simtemp: read\n");
    return 0;
}

/************************************
 * DRIVER SETUP
 ************************************/
module_init(nxp_simtemp_init);
module_exit(nxp_simtemp_exit);

/************************************
 * MODULE INFORMATION
 ************************************/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Carlos Zepeda");
MODULE_DESCRIPTION("Linux kernel driver to simulate a temperature sensor");