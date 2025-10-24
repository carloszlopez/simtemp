# NXP Simulated Temperature Platform Driver (`simtemp`)

A reference Linux **platform driver + user-space tools** that simulate a simple temperature sensor for NXP challenge purposes.

# Scripts
**scripts\build.sh**  
builds or cleans (if "clean" parameter is used) kernel\nxp_simtemp.ko and user\cli\nxp_simtemp_cli

*note: gui is build with the help of Qt Creator and user\gui\gui.pro project*
*note: scripts\nxp_simtemp.log is created after running build.sh*

**scripts\run_demo.sh**  
Demonstrate the kernel\nxp_simtemp.ko, user\cli\nxp_simtemp_cli and user\gui\build\Desktop_Qt_6_9_3-Debug\gui interaction
1. Inserts nxp_simtemp.ko
2. Runs nxp_simtemp_cli  
    - In "testmode"
    - In "read" 
3. Runs gui
4. Remove nxp_simtemp.ko

*note: for more information, please check the [DESIGN](DESIGN.md) documentation*

# Links
- Video presentation: https://drive.google.com/file/d/1DpQBfgv0GUhd_npHIchCipTYI6fHNXzn/view?usp=sharing
- Git repository: https://github.com/carloszlopez/simtemp.git
