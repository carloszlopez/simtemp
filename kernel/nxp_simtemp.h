#ifndef NXP_SIMTEMP_H
#define NXP_SIMTEMP_H

/************************************
 * INCLUDES
 ************************************/

 /************************************
 * DEFINE
 ************************************/
/* Time values */
#define SAMPLING_TIME (1000) /* Sampling time (1 sec) */

/* Event values */
#define EVENT_MASK_DEFAULT (0x0) /* Event mask DEFAULT value */
#define EVENT_MASK_TIMER (0x1) /* Event mask for time expired */
#define EVENT_MASK_TH (0x2) /* Event mask for time threshold crossed */

/* Temperature values */
#define TEMP_MIN (-70000) /* Min temperature to be simulated (-70 Celsius) */
#define TEMP_MAX (70000) /* Max temperature to be simulated (70 Celsius) */
#define TEMP_DELTA_MIN (-5000) /* Max temperature delta (-5 Celsius) */
#define TEMP_VALUE (25000) /* Default temperature value */
#define TEMP_THRESHOLD (10000) /* Default temperature threshold for event */
#define TEMP_DELTA_MAX (5000) /* Min temperature delta (-5 Celsius) */
#define TEMP_DELTA_RANGE (TEMP_DELTA_MAX - TEMP_DELTA_MIN + 1) /* Delta range */

#endif