CFLAGS =
LDFLAGS =

C_OBJECTS=misc.o sfork.o regex.o thash.o tlist.o xstr.o wstr.o mmatic.o tsort.o fifos.o
TARGETS=libasn.so

include rules.mk

pcre/.libs/libpcre.a:
	./pcre-build.sh

libasn.so: $(C_OBJECTS) pcre/.libs/libpcre.a
	$(CC) $(LDFLAGS) $(C_OBJECTS) -shared -o libasn.so pcre/.libs/libpcre.a
