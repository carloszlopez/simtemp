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
static ssize_t sampling_ms_show(struct device *dev,
                                struct device_attribute *attr,
                                char *buf);
static ssize_t sampling_ms_store(struct device *dev, 
                                 struct device_attribute *attr, 
                                 const char *buf, size_t count);
static ssize_t threshold_mC_show(struct device *dev,
                                struct device_attribute *attr,
                                char *buf);
static ssize_t threshold_mC_store(struct device *dev, 
                                 struct device_attribute *attr, 
                                 const char *buf, size_t count);
static ssize_t mode_show(struct device *dev, struct device_attribute *attr,
                        char *buf);
static ssize_t mode_store(struct device *dev, struct device_attribute *attr, 
                          const char *buf, size_t count);

/************************************
 * STATIC VARIABLES
 ************************************/
/* This variable is used to store the configuration */
static struct nxp_simtemp_cfg_t nxp_simtemp_cfg = {
    .sampling_ms = SAMPLING_TIME,
    .threshold_mC = TEMP_MAX,
    .mode = MODE_NORMAL
};
/* This variable is used to store the sampling timer */
static struct hrtimer nxp_simtemp_timer;
/* This variable is used to store the wait queue */
static wait_queue_head_t nxp_simtemp_wait 
    = __WAIT_QUEUE_HEAD_INITIALIZER(nxp_simtemp_wait);
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
/* This variable is used to store the sample values */
static struct nxp_simtemp_sample_t nxp_simtemp_sample = {
    .timestamp_ns = 0,
    .temp_mC = TEMP_VALUE,
    .flags = EVENT_MASK_DEFAULT,
};
/* This variable is used to store the sampling_ms device attribute */
static DEVICE_ATTR_RW(sampling_ms);
/* This variable is used to store the threshold_mC device attribute */
static DEVICE_ATTR_RW(threshold_mC);
/* This variable is used to store the mode device attribute */
static DEVICE_ATTR_RW(mode);

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

    /* Create fs */
    ret = device_create_file(nxp_simtemp_miscdev.this_device, 
        &dev_attr_sampling_ms);
    if (ret) return ret;
    ret = device_create_file(nxp_simtemp_miscdev.this_device, 
        &dev_attr_threshold_mC);
    if (ret) return ret;
    ret = device_create_file(nxp_simtemp_miscdev.this_device, 
        &dev_attr_mode);
    if (ret) return ret;

    /* Start timer */
    hrtimer_init(&nxp_simtemp_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    nxp_simtemp_timer.function = &nxp_simtemp_sample_cbk;
    hrtimer_start(&nxp_simtemp_timer, ms_to_ktime(nxp_simtemp_cfg.sampling_ms),
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
    printk("nxp_simtemp: Read start\n");
    
    /* Validate size */
    if (count < sizeof(nxp_simtemp_sample)) {
        printk("nxp_simtemp: incorrect size\n");
        return -EINVAL;
    }
    
    /* Copy record to userspace */
    if (copy_to_user(buf, &nxp_simtemp_sample, sizeof(nxp_simtemp_sample))) {
        printk("nxp_simtemp: copy to user error\n");
        return -EFAULT;
    }

    /* Clear events */
    nxp_simtemp_sample.flags = EVENT_MASK_DEFAULT;

    return sizeof(nxp_simtemp_sample);
}

/* Function used to simulate temperature */
static void nxp_simtemp_get_temp(void) {
    int temp = nxp_simtemp_sample.temp_mC; /* current temperature */
    /* Get a random temp in [TEMP_DELTA_MIN,TEMP_DELTA_MAX] range */
    int temp_delta = TEMP_DELTA_MIN + (get_random_u32() % TEMP_DELTA_RANGE);
    
    switch (nxp_simtemp_cfg.mode) {
    case MODE_NORMAL:
        /* Fixed stable value */
        temp = TEMP_VALUE;
        break;

    case MODE_NOISY:
        /* Around baseline + jitter */
        temp = TEMP_VALUE + temp_delta;
        break;

    case MODE_RAMP:
        /* Continuous drift from last value */
        temp += temp_delta;
        break;

    default:
        break;
    }

    nxp_simtemp_sample.temp_mC = clamp(temp, TEMP_MIN, TEMP_MAX);
    
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
    if (nxp_simtemp_sample.temp_mC >= nxp_simtemp_cfg.threshold_mC) {
        nxp_simtemp_sample.flags |= EVENT_MASK_TH;
    }

    /* Update timestamp */
    nxp_simtemp_sample.timestamp_ns = ktime_get_ns();

    /* Wake up any processes waiting in poll */
    wake_up_interruptible(&nxp_simtemp_wait);
    
    /* Restart timer */
    hrtimer_forward_now(&nxp_simtemp_timer, 
        ms_to_ktime(nxp_simtemp_cfg.sampling_ms));

    return HRTIMER_RESTART;
}

/* poll callback */
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

/* sampling_ms show callback */
static ssize_t sampling_ms_show(struct device *dev,
                                struct device_attribute *attr,
                                char *buf)
{
    return sprintf(buf, "%d\n", nxp_simtemp_cfg.sampling_ms);
}

/* sampling_ms store callback */
static ssize_t sampling_ms_store(struct device *dev, 
                                 struct device_attribute *attr, 
                                 const char *buf, size_t count) {
    int val;
    if (kstrtoint(buf, 10, &val))
        return -EINVAL;
    nxp_simtemp_cfg.sampling_ms = val;
    return count;
}

/* threshold_mC show callback */
static ssize_t threshold_mC_show(struct device *dev,
                                 struct device_attribute *attr,
                                 char *buf) {
    return sprintf(buf, "%d\n", nxp_simtemp_cfg.threshold_mC);
}

/* threshold_mC store callback */
static ssize_t threshold_mC_store(struct device *dev,
                                  struct device_attribute *attr,
                                  const char *buf, size_t count) {
    int val;
    if (kstrtoint(buf, 10, &val))
        return -EINVAL;

    nxp_simtemp_cfg.threshold_mC = val;
    return count;
}

/* mode show callback */
static ssize_t mode_show(struct device *dev, struct device_attribute *attr,
                        char *buf) {
    return sprintf(buf, "%d\n", nxp_simtemp_cfg.mode);
}

/* mode store callback */
static ssize_t mode_store(struct device *dev, struct device_attribute *attr, 
                          const char *buf, size_t count) {
    int val;

    if (kstrtoint(buf, 10, &val))
        return -EINVAL;

    if (val < 0 || val >= MODE_MAX)
        return -EINVAL;

    nxp_simtemp_cfg.mode = val;
    return count;
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