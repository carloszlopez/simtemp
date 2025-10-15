# Module name
obj-m += nxp_simtemp.o

# List of source files to include in the module
nxp_simtemp-objs := nxp_simtemp_core.o nxp_simtemp_buffer.o nxp_simtemp_sysfs.o

# Build directory for our module outputs

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(CURDIR) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(CURDIR) clean