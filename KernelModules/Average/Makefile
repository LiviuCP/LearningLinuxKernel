SRC_COMMON := source/Common/average_exit.c \
        source/Common/average_utilities.c

SRC_V1 := source/V1/average_init.c
SRC_V2 := source/V2/average_init.c

obj-m += average1.o
obj-m += average2.o

average1-objs := $(SRC_COMMON:.c=.o) $(SRC_V1:.c=.o)
average2-objs := $(SRC_COMMON:.c=.o) $(SRC_V2:.c=.o)

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

EXTRA_CFLAGS := -I$(src)/include -I$(src)/source
