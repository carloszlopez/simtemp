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
#include "nxp_simtemp.h"

/************************************
 * STATIC FUNCTION PROTOTYPES
 ************************************/
#if TEST_LOCAL_DEV_EN
static void nxp_simtemp_release(struct device *device);
#endif
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
static struct hrtimer nxp_simtemp_timer;

static int nxp_simtemp_poll_events;

static wait_queue_head_t nxp_simtemp_wait = __WAIT_QUEUE_HEAD_INITIALIZER(nxp_simtemp_wait);

static int nxp_simtemp_temp;

static int nxp_simtemp_sample_time;

static int nxp_simtemp_temp_tresh;

#if TEST_LOCAL_DEV_EN
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
    .poll = nxp_simtemp_poll,
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
#if TEST_LOCAL_DEV_EN
        /* Register device */   
        ret = platform_device_register(&nxp_simtemp_device);
        if (ret) {
            printk("nxp_simtemp: device register error\n");
        }
        else {
#endif
            /* Set params to default */
            nxp_simtemp_poll_events = EVENT_MASK_DEFAULT;
            nxp_simtemp_temp_tresh = TEMP_TRESHOLD;
            nxp_simtemp_sample_time = SAMPLING_TIME;
            nxp_simtemp_temp = TEMP_VALUE;
            /* Start timer */
            hrtimer_init(&nxp_simtemp_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
            nxp_simtemp_timer.function = &nxp_simtemp_sample_cbk;
            hrtimer_start(&nxp_simtemp_timer, 
                ms_to_ktime(nxp_simtemp_sample_time), 
                HRTIMER_MODE_REL);
#if TEST_LOCAL_DEV_EN
        }
#endif
    }
    return ret;
}

static void nxp_simtemp_exit(void) {
    printk("nxp_simtemp: exit\n");
    /* Cancel sampling timer */
    hrtimer_cancel(&nxp_simtemp_timer);
    /* Unregister driver and device */
    platform_driver_unregister(&nxp_simtemp_driver);
    platform_device_unregister(&nxp_simtemp_device);
}

#if TEST_LOCAL_DEV_EN
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
    char msg[32]; /* Temperature message */
    int msg_len; /* Temperature message lenght */
    
    printk("nxp_simtemp: Read start\n");
    if (EVENT_MASK_DEFAULT == nxp_simtemp_poll_events){
        /* Data is not ready */
        ret = -EAGAIN;
    } else {
        printk("nxp_simtemp: events: [timer=%d th=%d]\n",
            !!(nxp_simtemp_poll_events & EVENT_MASK_TIMER),
            !!(nxp_simtemp_poll_events & EVENT_MASK_TH));
        /* Send temperature to user */
        msg_len = snprintf(msg, sizeof(msg), "%d\n", nxp_simtemp_temp);
        if (msg_len > count) {
            msg_len = count;
        } else {
            /* do nothing */

        }
        if (copy_to_user(buf, msg, msg_len)) {
            ret = -EFAULT;
        } else {
            /* Clear events */
            nxp_simtemp_poll_events = EVENT_MASK_DEFAULT;
            ret = msg_len;
        }
    }
    return ret;
}

static void nxp_simtemp_get_temp(void) {
#if TEST_SIM_TEMP_EN
    int temp_delta; /* Temperature delta */
#endif

    /* Get temperature */
#if TEST_SIM_TEMP_EN
    temp_delta = TEMP_DELTA_MIN + (get_random_u32() % TEMP_DELTA_RANGE);
    printk("nxp_simtemp: temperature delta is: %d\n", temp_delta);
    nxp_simtemp_temp += temp_delta;
    if (TEMP_MIN > nxp_simtemp_temp) {
        nxp_simtemp_temp = TEMP_MIN;
    } else if (TEMP_MAX < nxp_simtemp_temp) {
        nxp_simtemp_temp = TEMP_MAX;
    } else {
        /* Do nothing */
    }
#endif
    printk("nxp_simtemp: temperature is: %d\n", nxp_simtemp_temp);
}

static enum hrtimer_restart nxp_simtemp_sample_cbk(struct hrtimer * timer) {
    printk("nxp_simtemp: sample callback\n");
    /* Get temperature */
    nxp_simtemp_get_temp();
    /* Check events */
    if (nxp_simtemp_temp_tresh < nxp_simtemp_temp) {
        nxp_simtemp_poll_events |= EVENT_MASK_TH;
    } else {
        /* do nothing */
    }
    nxp_simtemp_poll_events |= EVENT_MASK_TIMER;
    /* Restart timer */
    hrtimer_forward_now(&nxp_simtemp_timer, 
        ms_to_ktime(nxp_simtemp_sample_time));
    /* Wake up any processes waiting in poll */
    wake_up_interruptible(&nxp_simtemp_wait);

    return HRTIMER_RESTART;
}

static __poll_t nxp_simtemp_poll (struct file *file, 
                                  struct poll_table_struct *table) {
    __poll_t ret = 0;
    printk("nxp_simtemp: poll callback\n");
    poll_wait(file, &nxp_simtemp_wait, table);
    if (EVENT_MASK_DEFAULT == nxp_simtemp_poll_events) {
        /* Do nothing */
    } else {
        ret = POLLIN;
    }
    return ret;
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