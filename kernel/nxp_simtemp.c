/************************************
 * INCLUDES
 ************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include "nxp_simtemp.h"

/************************************
 * STATIC FUNCTION PROTOTYPES
 ************************************/
#if LOCAL_DEVICE
static int nxp_simtemp_init(void);
static void nxp_simtemp_exit(void);
#endif

/************************************
 * STATIC FUNCTIONS
 ************************************/
#if LOCAL_DEVICE
static int nxp_simtemp_init(void) {
    printk("nxp_simtemp: init\n");
    return 0;
}

static void nxp_simtemp_exit(void) {
    pr_info("nxp_simtemp: exit\n");
}
#endif

/************************************
 * DRIVER SETUP
 ************************************/
#if LOCAL_DEVICE
module_init(nxp_simtemp_init);
module_exit(nxp_simtemp_exit);
#endif

/************************************
 * MODULE INFORMATION
 ************************************/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Carlos Zepeda");
MODULE_DESCRIPTION("Linux kernel driver to simulate a temperature sensor");