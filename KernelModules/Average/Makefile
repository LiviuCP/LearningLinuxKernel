obj-m += average1.o
obj-m += average2.o

average1-objs := average1_init.o common.o average_exit.o utilities.o
average2-objs := average2_init.o common.o average_exit.o utilities.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
