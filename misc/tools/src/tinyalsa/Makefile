CFLAGS = -c -fPIC
INC = include
OBJECTS = mixer.o pcm.o
LIB = libtinyalsa.so

all: $(LIB) tinyplay tinycap tinymix

tinyplay: $(LIB) tinyplay.o
	$(CC) tinyplay.o -L. -ltinyalsa -o tinyplay

tinycap: $(LIB) tinycap.o
	$(CC) tinycap.o -L. -ltinyalsa -o tinycap

tinymix: $(LIB) tinymix.o
	$(CC) tinymix.o -L. -ltinyalsa -o tinymix

$(LIB): $(OBJECTS)
	$(CC) -shared $(OBJECTS) -o $(LIB)

.c.o:
	$(CC) $(CFLAGS) $< -I$(INC)
	
clean:
	-rm $(LIB) $(OBJECTS) tinyplay.o tinyplay tinycap.o tinycap \
	tinymix.o tinymix
