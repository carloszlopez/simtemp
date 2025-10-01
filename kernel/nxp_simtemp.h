#ifndef NXP_SIMTEMP_H
#define NXP_SIMTEMP_H

/************************************
 * INCLUDES
 ************************************/

 /************************************
 * DEFINE
 ************************************/
#define TEST_LOCAL_DEV_EN (1) /* 1 for enabling the local device creation */
#define TEST_SIM_TEMP_EN (1) /* 1 for enabling the temperature simulation */
#define TEST_SAMPLE_TIME_EN (1) /* 1 for enabling the sample time */
#if TEST_SAMPLE_TIME_EN
#define SAMPLING_TIME (1000) /* Sampling time (1 sec) */
#endif

#define TEMP_MIN (-70000) /* -70 Celsius */
#define TEMP_MAX (70000) /* 70 Celsius */

#define EVENT_MASK_DEFAULT (0x0) /* Event mask DEFAULT value */
#define EVENT_MASK_TIMER (0x1) /* Event mask for time expired */
#define EVENT_MASK_TH (0x2) /* Event mask for time threshold crossed */

#if TEST_LOCAL_DEV_EN
#define TEMP_DELTA_MIN (-5000) /* Max temperature delta (-5 Celsius) */
#define TEMP_DELTA_MAX (5000) /* Min temperature delta (-5 Celsius) */
#define TEMP_DELTA_RANGE (TEMP_DELTA_MAX - TEMP_DELTA_MIN + 1) /* Delta range */
#endif


#endif