CFLAGS =
LDFLAGS =

C_OBJECTS=lib.o sfork.o regex.o thash.o tlist.o xstr.o wstr.o mmatic.o tsort.o fifos.o fc.o
TARGETS=libasn.so libasn.a example

include rules.mk

pcre/.libs/libpcre.a:
	./pcre-build.sh

libasn.so: pcre/.libs/libpcre.a $(C_OBJECTS)
	$(CC) $(LDFLAGS) $(C_OBJECTS) -shared -o libasn.so pcre/.libs/libpcre.a

libasn.a: $(C_OBJECTS) pcre/.libs/libpcre.a
	$(AR) rc libasn.a $(C_OBJECTS) pcre/.libs/libpcre.a

example: example.o libasn.a pcre/.libs/libpcre.a
	$(CC) $(CFLAGS) example.c -o example libasn.a pcre/.libs/libpcre.a

install: all
	install -m 755 -d $(PKGDST)/include/libasn
	install -m 644 *.h $(PKGDST)/include/libasn
	install -m 755 -d $(PKGDST)/lib
	install -m 755 libasn.so $(PKGDST)/lib
