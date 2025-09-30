/************************************
 * INCLUDES
 ************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/random.h>
#include "nxp_simtemp.h"

/************************************
 * STATIC FUNCTION PROTOTYPES
 ************************************/
#if TEST_LOCAL_DEV
static void nxp_simtemp_release(struct device *device);
#endif
static int nxp_simtemp_get_temp(void);
static int nxp_simtemp_init(void);
static void nxp_simtemp_exit(void);
static int nxp_simtemp_probe(struct platform_device *drvrptr);
static void nxp_simtemp_remove(struct platform_device *drvrptr);
static ssize_t nxp_simtemp_read(struct file *filep, char __user *buf, 
                                size_t count, loff_t *offset);

/************************************
 * STATIC VARIABLES
 ************************************/
#if TEST_LOCAL_DEV
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
#if TEST_LOCAL_DEV
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

#if TEST_LOCAL_DEV
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
    int temp; /* Temperature */

    printk("nxp_simtemp: read\n");
    /* Get temperature */
    temp = nxp_simtemp_get_temp();
    return 0;
}

static int nxp_simtemp_get_temp(void) {
#if TEST_SIM_TEMP
    int rnd_num; /* Random 32-bit number */
#endif
    int temp = 25000; /* Temperature result */

    /* Get temperature */
#if TEST_SIM_TEMP
    /* Get random num and turn it into a temp value in [min,max] range */
    rnd_num = get_random_u32();
    temp = TEMP_MIN + (rnd_num % TEMP_RANGE);
    printk("nxp_simtemp: temperature is: %d\n", temp);
#endif
    return temp;
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