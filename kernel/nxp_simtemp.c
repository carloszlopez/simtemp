/************************************
 * INCLUDES
 ************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/random.h>
#include <linux/uaccess.h>
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
    if (ret) {
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
    ssize_t ret; /* Return result */
    int temp; /* Temperature */
    char msg[32]; /* Temperature message */
    int msg_len; /* Temperature message lenght */

    if (*offset > 0) {
        printk("nxp_simtemp: End of file reached\n");
        ret = 0;
    } else {
        /* Send temperature to user */
        printk("nxp_simtemp: Read start\n");
        temp = nxp_simtemp_get_temp();
        msg_len = snprintf(msg, sizeof(msg), "%d\n", temp);
        if (copy_to_user(buf, msg, msg_len)) {
            ret = -EFAULT;
        } else {
            ret = msg_len;
            *offset = msg_len;
        }
    }
    return ret;
}

static int nxp_simtemp_get_temp(void) {
#if TEST_SIM_TEMP
    int temp_delta; /* Temperature delta */
    static int sim_temp = 25000; /* simulated temperature */
#endif
    int temp; /* Temperature result */

    /* Get temperature */
#if TEST_SIM_TEMP
    temp_delta = TEMP_DELTA_MIN + (get_random_u32() % TEMP_DELTA_RANGE);
    printk("nxp_simtemp: temperature delta is: %d\n", temp_delta);
    sim_temp += temp_delta;
    if (TEMP_MIN > sim_temp) {
        sim_temp = TEMP_MIN;
    } else if (TEMP_MAX < sim_temp) {
        sim_temp = TEMP_MAX;
    } else {
        /* Do nothing */
    }
    printk("nxp_simtemp: temperature is: %d\n", sim_temp);
    temp = sim_temp;
#else
    temp = 25000;
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