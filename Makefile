CFLAGS =
LDFLAGS =

C_OBJECTS=misc.o sfork.o regex.o thash.o tlist.o xstr.o wstr.o mmatic.o tsort.o fifos.o
TARGETS=libasn.so libasn.a

include rules.mk

pcre/.libs/libpcre.a:
	./pcre-build.sh

libasn.so: pcre/.libs/libpcre.a $(C_OBJECTS)
	$(CC) $(LDFLAGS) $(C_OBJECTS) -shared -o libasn.so pcre/.libs/libpcre.a

libasn.a: $(C_OBJECTS) pcre/.libs/libpcre.a
	$(AR) rc libasn.a $(C_OBJECTS) pcre/.libs/libpcre.a
