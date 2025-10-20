#ifndef NXP_SIMTEMP_H
#define NXP_SIMTEMP_H

/************************************
 * Includes
 ************************************/
#include <linux/miscdevice.h>
#include <linux/poll.h>
#include <linux/uaccess.h>
#include "nxp_simtemp_contract.h"

/************************************
 * Variables
 ************************************/
extern struct miscdevice nxp_simtemp_miscdev;
extern struct nxp_simtemp_sysfs_t nxp_simtemp_sysfs;

/************************************
 * Functions
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