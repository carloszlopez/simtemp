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

#define TEMP_MIN (-70000) /* -70 C in mCelsius */
#define TEMP_MAX (70000) /* -70 C in mCelsius */
#define TEMP_RANGE (TEMP_MAX - TEMP_MIN + 1) /* Int values in [MIN,MAX] range */

#endif