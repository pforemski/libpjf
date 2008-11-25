CFLAGS =
LDFLAGS =

ME=libasn
C_OBJECTS=lib.o sfork.o regex.o thash.o tlist.o xstr.o wstr.o mmatic.o tsort.o fifos.o fc.o select.o
TARGETS=libasn.so libasn.a libasn_example

include rules.mk

pcre/.libs/libpcre.a:
	./pcre-build.sh

libasn.so: pcre/.libs/libpcre.a $(C_OBJECTS)
	$(CC) $(LDFLAGS) $(C_OBJECTS) -shared -o libasn.so pcre/.libs/libpcre.a

libasn.a: $(C_OBJECTS) pcre/.libs/libpcre.a
	$(AR) rc libasn.a $(C_OBJECTS) pcre/.libs/libpcre.a

libasn_example: libasn_example.o libasn.a pcre/.libs/libpcre.a
	$(CC) $(CFLAGS) libasn_example.c -o libasn_example libasn.a pcre/.libs/libpcre.a

install: install-std
