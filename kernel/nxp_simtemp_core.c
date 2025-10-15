/************************************
 * Includes
 ************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include "nxp_simtemp.h"

/************************************
 * Internal functions prototypes
 ************************************/
static void nxp_simtemp_release(struct device *device);
static int nxp_simtemp_probe(struct platform_device *drvrptr);
static void nxp_simtemp_remove(struct platform_device *drvrptr);

/************************************
 * Internal variables
 ************************************/
/* This variable is used to store the platform device */
static struct platform_device nxp_simtemp_device = {
    .name = "nxp_simtemp",
    .id = -1,
    .dev = {
        .release = nxp_simtemp_release,
    },
};
/* This variable is used to store the platform driver */
static struct platform_driver nxp_simtemp_driver = {
    .probe = nxp_simtemp_probe,
    .remove = nxp_simtemp_remove,
    .driver = {
        .name = "nxp_simtemp",
        .owner = THIS_MODULE,
    },
};
/* This variable is used to store the character device operations */
static const struct file_operations nxp_simtemp_fops = {
    .owner = THIS_MODULE,
    .read  = nxp_simtemp_buffer_read,
    .poll = nxp_simtemp_buffer_poll,
};

/************************************
 * External variables
 ************************************/
/* This variable is used to store the character device */
struct miscdevice nxp_simtemp_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "nxp_simtemp",
    .fops = &nxp_simtemp_fops,
};

/************************************
 * Internal functions
 ************************************/
/* Init module function */
static int nxp_simtemp_init(void) {    
    printk("nxp_simtemp: init\n");

    /* Register driver */
    int ret = platform_driver_register(&nxp_simtemp_driver);
    if (ret) {
        printk("nxp_simtemp: error on platform driver registration\n");
        return ret;
    }
    
    /* Register device */   
    ret = platform_device_register(&nxp_simtemp_device);
    if (ret) {
        printk("nxp_simtemp: error on platform device registration\n");
        return ret;
    }
    
    return 0;
}

/* Exit module function */
static void nxp_simtemp_exit(void) {
    printk("nxp_simtemp: exit\n");
    
    /* Unregister device */
    platform_device_unregister(&nxp_simtemp_device);
    
    /* Unregister driver */
    platform_driver_unregister(&nxp_simtemp_driver);
}

/* Device release */
static void nxp_simtemp_release(struct device *device) {
    printk("nxp_simtemp: release\n");
}

/* Probe device function */
static int nxp_simtemp_probe(struct platform_device *device) {
    printk("nxp_simtemp: release\n");

    /* Register character device */
    int ret = misc_register(&nxp_simtemp_miscdev);
    if (ret) {
        printk("nxp_simtemp: error on character device registration\n");
        return ret;
    }

    /* Sysfs init */
    ret = nxp_simtemp_sysfs_init();
    if (ret) return ret;

    /* Buffer init */
    ret = nxp_simtemp_buffer_init();
    return ret;
}

/* Remove device function */
static void nxp_simtemp_remove(struct platform_device *device) {
    printk("nxp_simtemp: remove\n");
    
    /* Buffer deinit */
    nxp_simtemp_buffer_deinit();

    /* Deregister character device */
    misc_deregister(&nxp_simtemp_miscdev);
}

/************************************
 * Module setup
 ************************************/
module_init(nxp_simtemp_init);
module_exit(nxp_simtemp_exit);

/************************************
 * Module information
 ************************************/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Carlos Zepeda");
MODULE_DESCRIPTION("Linux kernel driver to simulate a temperature sensor");