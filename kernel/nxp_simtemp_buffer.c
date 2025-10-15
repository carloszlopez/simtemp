/************************************
 * Includes
 ************************************/
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/minmax.h>
#include <linux/types.h>
#include <linux/random.h>
#include "nxp_simtemp.h"

/************************************
 * Enumerations
 ************************************/
/* Represents the events in sample.flags */
enum event_t {
    NO_EVENT = 0x0, /* Event mask DEFAULT value */
    TIMER_EVENT = 0x1, /* Event mask for time expired */
    THRESHOLD_EVENT =  0x2, /* Event mask for time threshold crossed */
};

/* Represents temperature delta values in miliCelsius */
enum delta_temp_t {
    TEMP_DELTA_MIN = -5000, /* Max temperature delta */
    TEMP_DELTA_MAX = 5000, /* Min temperature delta */
    TEMP_DELTA_RANGE = (TEMP_DELTA_MAX - TEMP_DELTA_MIN + 1), /* Delta range */
};

/************************************
 * Structs
 ************************************/
/* Represents the sample values */
struct sample_t {
    __u64 timestamp_ns;   /* monotonic timestamp */
    __s32 temp_mC;        /* milli-degree Celsius */
    __u32 flags;          /* Events */
} __attribute__((packed));

/************************************
 * Internal variables
 ************************************/
/* Holds the timer used to generate a new sample */
static struct hrtimer sampling_timer;

/* Holds the wait queue */
static wait_queue_head_t wait_queue = __WAIT_QUEUE_HEAD_INITIALIZER(wait_queue);

/* Holds the sample values */
static struct sample_t sample = {
    .timestamp_ns = 0,
    .temp_mC = TEMP_INIT,
    .flags = NO_EVENT,
};

/************************************
 * Internal functions
 ************************************/
/* Used to generate a new temperature sample (simulated) */
static void update_temperature(void) {
    int temp = sample.temp_mC; /* current temperature */
    /* Get a random temp in [TEMP_DELTA_MIN,TEMP_DELTA_MAX] range */
    int temp_delta = TEMP_DELTA_MIN + (get_random_u32() % TEMP_DELTA_RANGE);
    
    switch (nxp_simtemp_sysfs.mode) {
    case MODE_NORMAL:
        /* Fixed stable value */
        temp = TEMP_INIT;
        break;

    case MODE_NOISY:
        /* Around baseline + jitter */
        temp = TEMP_INIT + temp_delta;
        break;

    case MODE_RAMP:
        /* Continuous drift from last value */
        temp += temp_delta;
        break;

    default:
        break;
    }

    sample.temp_mC = clamp(temp, TEMP_MIN, TEMP_MAX);
    
    printk("nxp_simtemp: temperature is: %d\n", sample.temp_mC);
}

/* Timer callback function */
static enum hrtimer_restart sampling_timer_callback(struct hrtimer * timer) {
    printk("nxp_simtemp: timer callback\n");

    update_temperature();

    /* Update timer flag */
    sample.flags |= TIMER_EVENT;

    /* Update threshold flag */
    if (sample.temp_mC >= nxp_simtemp_sysfs.threshold_mC)
        sample.flags |= THRESHOLD_EVENT;

    /* Update timestamp */
    sample.timestamp_ns = ktime_get_ns();

    /* Wake up any processes waiting in poll */
    wake_up_interruptible(&wait_queue);
    
    /* Restart timer */
    hrtimer_forward_now(timer, ms_to_ktime(nxp_simtemp_sysfs.sampling_ms));

    return HRTIMER_RESTART;
}

/************************************
 * External functions
 ************************************/
/* Init function */
int nxp_simtemp_buffer_init(void) {
    /* Init timer */
    hrtimer_init(&sampling_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

    /* Assign timer callback */
    sampling_timer.function = &sampling_timer_callback;
    
    /* Start timer */
    hrtimer_start(&sampling_timer, ms_to_ktime(nxp_simtemp_sysfs.sampling_ms),
        HRTIMER_MODE_REL);

    return 0;
}

/* Deinit function */
void nxp_simtemp_buffer_deinit(void) {
    /* Cancel timer */
    hrtimer_cancel(&sampling_timer);
}

/* Device read function */
ssize_t nxp_simtemp_buffer_read(struct file *filep, char __user *buf, 
                                size_t count, loff_t *offset) {
    printk("nxp_simtemp: device read\n");
    
    /* Validate size */
    if (count < sizeof(sample)) {
        printk("nxp_simtemp: incorrect size\n");
        return -EINVAL;
    }
    
    /* Copy record to userspace */
    if (copy_to_user(buf, &sample, sizeof(sample))) {
        printk("nxp_simtemp: copy to user error\n");
        return -EFAULT;
    }

    /* Clear events */
    sample.flags = NO_EVENT;

    return sizeof(sample);
}

/* poll callback */
__poll_t nxp_simtemp_buffer_poll (struct file *file, 
                                 struct poll_table_struct *table) {
    printk("nxp_simtemp: poll callback\n");
    
    /* Wait for wait_queue */
    poll_wait(file, &wait_queue, table);
    
    /* Check if data is ready */
    if (sample.flags != NO_EVENT) {
        return POLLIN;
    }

    return 0;
}