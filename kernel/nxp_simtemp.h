#ifndef NXP_SIMTEMP_H
#define NXP_SIMTEMP_H

/************************************
 * INCLUDES
 ************************************/

 /************************************
 * DEFINE
 ************************************/
#define TEST_LOCAL_DEV (1) /* 1 for enabling the local device creation */
#define TEST_SIM_TEMP (1) /* 1 for enabling the temperature simulation */

#define TEMP_MIN (-70000) /* -70 Celsius */
#define TEMP_MAX (70000) /* 70 Celsius */

#if TEST_LOCAL_DEV
#define TEMP_DELTA_MIN (-5000) /* Max temperature delta (-5 Celsius) */
#define TEMP_DELTA_MAX (5000) /* Min temperature delta (-5 Celsius) */
#define TEMP_DELTA_RANGE (TEMP_DELTA_MAX - TEMP_DELTA_MIN + 1) /* Delta range */
#endif

#endif