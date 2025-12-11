obj-m += division.o

division-objs := division_main.o division_impl.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

EXTRA_CFLAGS := -I$(src)/include
