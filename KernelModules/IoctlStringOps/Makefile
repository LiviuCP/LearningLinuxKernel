SRC := source/ioctl_string_ops_impl.c source/ioctl_string_ops_main.c

obj-m += ioctl_string_ops.o

ioctl_string_ops-objs := $(SRC:.c=.o)

KERNEL_UTILITIES_BUILD_DIR := $(shell cd $(BUILD_DIR) && cd ../KernelUtilities && echo `pwd`)
KBUILD_EXTRA_SYMBOLS += $(KERNEL_UTILITIES_BUILD_DIR)/Module.symvers

# the BUILD_DIR variable should be provided from project (CMake), not set manually by user
BUILD_DIR_MAKEFILE := $(BUILD_DIR)/Makefile

all: $(BUILD_DIR_MAKEFILE)
	make -C /lib/modules/$(shell uname -r)/build M=$(BUILD_DIR) src=$(PWD) modules

$(BUILD_DIR):
	mkdir -p "$@"

ADD_EXTRA_SYMBOLS := "KBUILD_EXTRA_SYMBOLS +="
ADD_EXTRA_SYMBOLS += $(KERNEL_UTILITIES_BUILD_DIR)/Module.symvers

$(BUILD_DIR_MAKEFILE): $(BUILD_DIR)
	echo $(ADD_EXTRA_SYMBOLS) > "$@"

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(BUILD_DIR) src=$(PWD) clean

EXTRA_CFLAGS := -I$(src)/include -I$(src)/source
