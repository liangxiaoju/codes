ifdef KERNELRELEASE

obj-m := imtsrv.o

else

all: imtsrv-genl imtsrv_ko

imtsrv-genl:
	gcc -o imtsrv-genl imtsrv-genl.c -I/usr/include/libnl3 -lnl-cli-3 -lnl-genl-3 -lnl-nf-3 -lnl-route-3 -lnl-3 

imtsrv_ko:
	make -C /usr/src/linux M=$(PWD) modules

clean:
	rm -f *.o *.mod.* *.order *.symvers

endif
