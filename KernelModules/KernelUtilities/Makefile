obj-m += kernelutilities.o

ifndef BUILD_DIR
	BUILD_DIR := $(PWD)/build
endif

# the BUILD_DIR variable should be provided from project (CMake), not set manually by user
BUILD_DIR_MAKEFILE := $(BUILD_DIR)/Makefile

all: $(BUILD_DIR_MAKEFILE)
	make -C /lib/modules/$(shell uname -r)/build M=$(BUILD_DIR) src=$(PWD) modules

$(BUILD_DIR):
	mkdir -p "$@"

$(BUILD_DIR_MAKEFILE): $(BUILD_DIR)
	touch "$@"

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(BUILD_DIR) src=$(PWD) clean
