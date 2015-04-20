prog = camera
SRC = camera.c fb.c utils.c
CFLAGS = -static

ALL: arm pc

pc : $(SRC)
	gcc $(CFLAGS) $(SRC) -o $(prog)

arm : $(SRC)
	arm-unknown-linux-gnueabi-gcc $(CFLAGS) $(SRC) -o $(prog).arm

clean:
	rm $(prog) $(prog).arm
