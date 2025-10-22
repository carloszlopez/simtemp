# NXP SIMTEMP DESIGN.

## Overview
The nxp_simtemp [driver](#driver-glossary) simulates a temperature sensor and exposes data through a character [device](#device-glossary) and [sysfs](#sysfs-glossary).  

## Modules
nxp_simtemp is split into core, sysfs and buffer:  
* **core** implements the [Driver](#driver-glossary) init/exit and [device](#device-glossary) probe/remove/release.  
*Also register a character device for testing.*  
* **sysfs** for the attrbiutes implementation.  
* **buffer** implements the [fops](#fops-glossary) and how the sensor generates the data.

## Contract
[Contract](#contract-glossary) is defined in ..\kernel\nxp_simtemp_contract.h

### Constant values
| Constant | Value | Description |
| ------- | ----------- | ----------- |
| TIMER_INIT | 1000 | [sampling_ms](#sampling_ms-glossary) initial value (1,000 mSec) |
| MODE_NORMAL | 0 | Normal or default [mode](#mode-glossary), temperature is fixed to a [baseline](#TEMP_BASE-glossary) |
| MODE_NOISY | 1 | Noisy [mode](#mode-glossary), sensor generates small random fluctuations around [baseline](#TEMP_BASE-glossary) |
| MODE_RAMP | 2 | Ramp [mode](#mode-glossary), sensor increase/decrease temperature from last value ([baseline](#TEMP_BASE-glossary), if first iteration) |
| <a id="TEMP_BASE-glossary"></a>TEMP_BASE | 25000 | Base temperature (25 °C) |
| TEMP_MAX | 70000 | Maximum temperature that sensor will generate (70,000 m°C). Also [threshold_mC](#threshold_mC-glossary) initial value |
| TEMP_MIN | -70000 | Minimum temperature that sensor will generate (-70,000 m°C)|
| NO_EVENT | 0 | Default or clear [flags](#flags-glossary) value |
| TIMER_EVENT | 1 | Event in [flags](#flags-glossary) to indicate [sampling_ms](#sampling_ms-glossary) time expired |
| <a id="THRESHOLD_EVENT-glossary"></a>THRESHOLD_EVENT | 2 | Event in [flags](#flags-glossary) to indicate sensor generated a temperature above [threshold_mC](#threshold_mC-glossary) |

### SYSFS
| Attrbitute | Perms | Description  |
| ------- | ----------- | ----------- |
| <a id="sampling_ms-glossary"></a>sampling_ms | R/W | Timer value for sensor to generate a new sample |
| <a id="threshold_mC-glossary"></a>threshold_mC | R/W | Temperature limit in m°C for setting [THRESHOLD_EVENT](#THRESHOLD_EVENT-glossary) |
| mode | R/W | Sensor mode |

~~stats (R) -~~ 

### DEV
Reading /dev/nxp_simtemp returns a binary record (sample) with below:
| Name | Bits | Description  |
| ------- | ----------- | ----------- |
| timestamp_ns | 64 | Monotonic timestamp in nano-secs |
| temp_mC | 32 | Temperature value in m°C |
| <a id="flags-glossary"></a>flags | 32 | Events flags |

## Diagrams
### Read sequence
This sequence diagram shows a basic blocking read interaction between [user](#user-glossary) and nxp_simtemp [driver](#driver-glossary)  
![read-seq-diagram](<Read sequence.jpg>)

## User
Users can interact with the nxp_simtemp [driver](#driver-glossary) using the user\cli\device.cpp class.

### CLI commands
| Name | Description  |
| ------- | ----------- |
|read | Reading loop |
|testmode | Sets a low temperature threshold and checks if flag is raised within 2 cycles |
|readattr x | Reads x attribute |
|writeattr x value | Writes value into x attribute  |

### GUI design  
**nxp_simtemp**  
main window, for showing current temperature, time stamp and threshold alert.  
**Settings**  
Settings window reads the [sysfs](#sysfs-glossary) and writes the values to them when "OK" button is pressed.   
![GUI design](<GUI design.jpg>)

## Glosary
| Concept | Description |
| ------- | ----------- |
| <a id="driver-glossary"></a>Driver | Linux module |
| <a id="device-glossary"></a>Device | Linux device |
| <a id="modules-glossary"></a>Modules | .c files |
| <a id="sysfs-glossary"></a>Sysfs | System filesystem |
| <a id="fops-glossary"></a>Fops | File operations |
| <a id="user-glossary"></a>User | Process running in user space |
| <a id="contract-glossary"></a>Contract | Kernel-user communication contract |