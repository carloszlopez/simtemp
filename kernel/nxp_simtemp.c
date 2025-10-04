/************************************
 * INCLUDES
 ************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/random.h>
#include <linux/uaccess.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/poll.h>
#include <linux/minmax.h>
#include <linux/types.h>
#include "nxp_simtemp.h"

/************************************
 * STATIC FUNCTION PROTOTYPES
 ************************************/
static void nxp_simtemp_release(struct device *device);
static void nxp_simtemp_get_temp(void);
static int nxp_simtemp_init(void);
static void nxp_simtemp_exit(void);
static int nxp_simtemp_probe(struct platform_device *drvrptr);
static void nxp_simtemp_remove(struct platform_device *drvrptr);
static ssize_t nxp_simtemp_read(struct file *filep, char __user *buf, 
                                size_t count, loff_t *offset);
static enum hrtimer_restart nxp_simtemp_sample_cbk(struct hrtimer * timer);
static __poll_t nxp_simtemp_poll (struct file *file, 
                                  struct poll_table_struct *table);

/************************************
 * STATIC VARIABLES
 ************************************/
/* This variable is used to store the temperature threshold */
static int nxp_simtemp_temp_tresh = TEMP_THRESHOLD;
/* This variable is used to store the sampling time */
static int nxp_simtemp_sample_time = SAMPLING_TIME;
/* This variable is used to store the sampling timer */
static struct hrtimer nxp_simtemp_timer;
/* This variable is used to store the wait queue */
static wait_queue_head_t nxp_simtemp_wait = __WAIT_QUEUE_HEAD_INITIALIZER(nxp_simtemp_wait);
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
    .read  = nxp_simtemp_read,
    .poll = nxp_simtemp_poll,
};
/* This variable is used to store the character device */
static struct miscdevice nxp_simtemp_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "nxp_simtemp",
    .fops = &nxp_simtemp_fops,
};
static struct nxp_simtemp_sample_t nxp_simtemp_sample = {
    .timestamp_ns = 0,
    .temp_mC = TEMP_VALUE,
    .flags = EVENT_MASK_DEFAULT,
};

/************************************
 * STATIC FUNCTIONS
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

    /* Start timer */
    hrtimer_init(&nxp_simtemp_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    nxp_simtemp_timer.function = &nxp_simtemp_sample_cbk;
    hrtimer_start(&nxp_simtemp_timer, ms_to_ktime(nxp_simtemp_sample_time),
        HRTIMER_MODE_REL);
    return 0;
}

/* Remove device function */
static void nxp_simtemp_remove(struct platform_device *device) {
    printk("nxp_simtemp: remove\n");
    /* Cancel timer */
    hrtimer_cancel(&nxp_simtemp_timer);
    /* Deregister character device */
    misc_deregister(&nxp_simtemp_miscdev);
}

/* Read function for character device */
static ssize_t nxp_simtemp_read(struct file *filep, char __user *buf, 
                                size_t count, loff_t *offset) {
    char msg[32]; /* Temperature message */
    
    printk("nxp_simtemp: Read start\n");

    /* Clear events */
    nxp_simtemp_sample.flags = EVENT_MASK_DEFAULT;

    /* Format temperature as string */
    int msg_len = snprintf(msg, sizeof(msg), "%d\n", nxp_simtemp_sample.temp_mC);
    /* Clamp msg len to requested len */
    if (msg_len > count) {
        msg_len = count;
    }
    /* Send msg to user */
    if (copy_to_user(buf, msg, msg_len)) {
        return -EFAULT;
    }
    return msg_len;
}

/* Function used to simulate temperature */
static void nxp_simtemp_get_temp(void) {
    /* Get a random temp in [TEMP_DELTA_MIN,TEMP_DELTA_MAX] range */
    int temp_delta = TEMP_DELTA_MIN + (get_random_u32() % TEMP_DELTA_RANGE);

    /* Calculate new temperature based on temp_delta */
    nxp_simtemp_sample.temp_mC += temp_delta;
    
    /* Clamp temperature to [TEMP_MIN,TEMP_MAX] range */
    nxp_simtemp_sample.temp_mC = clamp(nxp_simtemp_sample.temp_mC, TEMP_MIN, TEMP_MAX);
    
    printk("nxp_simtemp: temperature is: %d\n", nxp_simtemp_sample.temp_mC);
}

/* Callback for temperature sample */
static enum hrtimer_restart nxp_simtemp_sample_cbk(struct hrtimer * timer) {
    printk("nxp_simtemp: sample callback\n");

    /* Get temperature */
    nxp_simtemp_get_temp();

    /* Set time event */
    nxp_simtemp_sample.flags |= EVENT_MASK_TIMER;
    /* Set threshold event if temperature is avobe threshold */
    if (nxp_simtemp_sample.temp_mC >= nxp_simtemp_temp_tresh) {
        nxp_simtemp_sample.flags |= EVENT_MASK_TH;
    }

    /* Wake up any processes waiting in poll */
    wake_up_interruptible(&nxp_simtemp_wait);
    
    /* Restart timer */
    hrtimer_forward_now(&nxp_simtemp_timer, 
        ms_to_ktime(nxp_simtemp_sample_time));

    return HRTIMER_RESTART;
}

static __poll_t nxp_simtemp_poll (struct file *file, 
                                  struct poll_table_struct *table) {
    printk("nxp_simtemp: poll callback\n");
    
    /* Wait for nxp_simtemp_wait */
    poll_wait(file, &nxp_simtemp_wait, table);
    
    /* Check if data is ready */
    if (nxp_simtemp_sample.flags != EVENT_MASK_DEFAULT) {
        return POLLIN;
    }

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