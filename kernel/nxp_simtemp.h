#ifndef NXP_SIMTEMP_H
#define NXP_SIMTEMP_H

/************************************
 * Includes
 ************************************/
#include <linux/miscdevice.h>
#include <linux/poll.h>
#include <linux/uaccess.h>

 /************************************
 * Enumerations
 ************************************/
/* Temperature values in miliCelsius */
enum nxp_simtemp_temp_t {
    TEMP_MIN = -70000, /* Minimum temperature */
    TEMP_INIT = 25000, /* Initial temperature value */
    TEMP_MAX = 70000, /* Maximum temperature and default threshold */
};
/* Temperature simulation mode values */
enum nxp_simtemp_mode_t {
    MODE_NORMAL = 0, /* Fixed/stable temperature */
    MODE_NOISY, /* Small random fluctuations around baseline */
    MODE_RAMP, /* Continuous drift (increase/decrease) */
    MODE_MAX, /* Max number of modes */
};

 /************************************
 * Structs
 ************************************/
/* Represents the sysfs attributes */
struct nxp_simtemp_sysfs_t {
    int sampling_ms; /* update period */
    int threshold_mC; /* alert threshold in milli‑°C */
    int mode; /* e.g., normal|noisy|ramp */
};

/************************************
 * Variables
 ************************************/
extern struct miscdevice nxp_simtemp_miscdev;
extern struct nxp_simtemp_sysfs_t nxp_simtemp_sysfs;

/************************************
 * functions
 ************************************/
int nxp_simtemp_buffer_init(void);
void nxp_simtemp_buffer_deinit(void);
ssize_t nxp_simtemp_buffer_read(struct file *filep, char __user *buf, 
                                size_t count, loff_t *offset);
__poll_t nxp_simtemp_buffer_poll (struct file *file, 
                                 struct poll_table_struct *table);

int nxp_simtemp_sysfs_init(void);
void nxp_simtemp_sysfs_deinit(void);
#endif