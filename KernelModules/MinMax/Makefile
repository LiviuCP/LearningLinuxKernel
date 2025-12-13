obj-m += minMax.o

#BUILD_DIR := $(PWD)/build
#BUILD_DIR_MAKEFILE := $(BUILD_DIR)/Makefile
MY_DIR_MAKEFILE := $(MY_DIR)/Makefile

all: $(MY_DIR_MAKEFILE)
	make -C /lib/modules/$(shell uname -r)/build M=$(MY_DIR) src=$(PWD) modules

$(MY_DIR):
	mkdir -p "$@"

$(MY_DIR_MAKEFILE): $(MY_DIR)
	touch "$@"

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(MY_DIR) src=$(PWD) clean
