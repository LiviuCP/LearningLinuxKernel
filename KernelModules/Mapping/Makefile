SRC := source/mapping_impl.c source/mapping_main.c source/mapping_utils.c

obj-m += mapping.o

mapping-objs := $(SRC:.c=.o)

ifndef BUILD_DIR
	BUILD_DIR := $(PWD)/build
endif

BUILD_DIR_MAKEFILE := $(BUILD_DIR)/Makefile

all: $(BUILD_DIR_MAKEFILE)
	make -C /lib/modules/$(shell uname -r)/build M=$(BUILD_DIR) src=$(PWD) modules

$(BUILD_DIR):
	mkdir -p "$@"

$(BUILD_DIR_MAKEFILE): $(BUILD_DIR)
	touch "$@"

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(BUILD_DIR) src=$(PWD) clean

EXTRA_CFLAGS := -I$(src)/include -I$(src)/source
