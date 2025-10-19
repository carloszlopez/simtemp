#ifndef DEVICE_H
#define DEVICE_H

 /************************************
 * Includes
 ************************************/
#include <linux/types.h>

 /************************************
 * Defines
 ************************************/
#define TIMER_INIT	(1000) /* sampling_ms initial value (1,000 mSec) */
#define MODE_NORMAL	(0)	/* Normal or default mode, temperature is fixed to a baseline */
#define MODE_NOISY	(1)	/* Noisy mode, sensor generates small random fluctuations around baseline */
#define MODE_RAMP	(2)	/* Ramp mode, sensor increase/decrease temperature from last value (baseline, if first iteration) */
#define TEMP_BASE	(25000)	/* Base temperature (25 °C) */
#define TEMP_MAX	(70000)	/* Maximum temperature that sensor will generate (70,000 m°C). Also threshold_mC initial value */
#define TEMP_MIN	(-70000) /* Minimum temperature that sensor will generate (-70,000 m°C) */
#define NO_EVENT	(0)	/* Default or clear flags value */
#define TIMER_EVENT	(1)	/* Event in flags to indicate sampling_ms time expired */
#define THRESHOLD_EVENT	(2)	/* Event in flags to indicate sensor generated a temperature above threshold_mC */

 /************************************
 * Structs
 ************************************/
/* Represents the binary record (sample) */
struct nxp_simtemp_sample_t {
    __u64 timestamp_ns;   /* Monotonic timestamp in nano-secs */
    __s32 temp_mC;        /* Temperature value in m°C */
    __u32 flags;          /* Events flags */
} __attribute__((packed));

/* Represents the sysfs attributes */
struct nxp_simtemp_sysfs_t {
    int sampling_ms; /* Timer value for sensor to generate a new sample */
    int threshold_mC; /* Temperature limit in m°C for setting THRESHOLD_EVENT */
    int mode; /* Sensor mode */
};
#endif