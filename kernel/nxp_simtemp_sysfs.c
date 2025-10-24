/************************************
 * Includes
 ************************************/
#include "nxp_simtemp.h"

/************************************
 * External variables
 ************************************/
/* Holds the value of the sysfs attributes */
struct nxp_simtemp_sysfs_t nxp_simtemp_sysfs = {
    .sampling_ms = TIMER_INIT,
    .threshold_mC = TEMP_MAX,
    .mode = MODE_NORMAL
};

/************************************
 * Show/Store functions
 ************************************/
/* sampling_ms */
static ssize_t sampling_ms_show(struct device *dev,
                                struct device_attribute *attr,
                                char *buf) {
    return sprintf(buf, "%d\n", nxp_simtemp_sysfs.sampling_ms);
}
static ssize_t sampling_ms_store(struct device *dev, 
                                 struct device_attribute *attr, 
                                 const char *buf, size_t count) {
    int val; /* sampling time value */

    /* Error getting int value */
    if (kstrtoint(buf, 10, &val))
        return -EINVAL;
    
    /* Incorrect value */
    if (val <= 0)
        return -EINVAL;

    nxp_simtemp_sysfs.sampling_ms = val;
    return count;
}

/* threshold_mC */
static ssize_t threshold_mC_show(struct device *dev,
                                 struct device_attribute *attr,
                                 char *buf) {
    return sprintf(buf, "%d\n", nxp_simtemp_sysfs.threshold_mC);
}
static ssize_t threshold_mC_store(struct device *dev,
                                  struct device_attribute *attr,
                                  const char *buf, size_t count) {
    int val; /* threshold value */

    /* Error getting int value */
    if (kstrtoint(buf, 10, &val))
        return -EINVAL;

    /* Incorrect value */
    if (val < TEMP_MIN || val > TEMP_MAX)
        return -EINVAL;

    nxp_simtemp_sysfs.threshold_mC = val;
    return count;
}

/* mode */
static ssize_t mode_show(struct device *dev, struct device_attribute *attr,
                        char *buf) {
    return sprintf(buf, "%d\n", nxp_simtemp_sysfs.mode);
}
static ssize_t mode_store(struct device *dev, struct device_attribute *attr, 
                          const char *buf, size_t count) {
    int val; /* mode value */

    /* Error getting int value */
    if (kstrtoint(buf, 10, &val))
        return -EINVAL;

    /* Incorrect value */
    if ((val < MODE_NORMAL) || (val > MODE_RAMP))
        return -EINVAL;

    nxp_simtemp_sysfs.mode = val;
    return count;
}

/************************************
 * Attributes variables
 ************************************/
static DEVICE_ATTR_RW(sampling_ms);
static DEVICE_ATTR_RW(threshold_mC);
static DEVICE_ATTR_RW(mode);

static struct attribute *nxp_simtemp_attrs[] = {
    &dev_attr_sampling_ms.attr,
    &dev_attr_threshold_mC.attr,
    &dev_attr_mode.attr,
    NULL,   /* array must be NULL-terminated */
};

static struct attribute_group nxp_simtemp_attr_group = {
    .attrs = nxp_simtemp_attrs,
};

/************************************
 * External functions
 ************************************/
/* Sysfs init function */
int nxp_simtemp_sysfs_init(void) {
    return sysfs_create_group(&nxp_simtemp_miscdev.this_device->kobj, 
                             &nxp_simtemp_attr_group);
}

/* Sysfs deinit function */
void nxp_simtemp_sysfs_deinit(void) {
    sysfs_remove_group(&nxp_simtemp_miscdev.this_device->kobj,
                       &nxp_simtemp_attr_group);
}