CFLAGS =
LDFLAGS =

C_OBJECTS=lib.o sfork.o regex.o thash.o tlist.o xstr.o wstr.o mmatic.o tsort.o fifos.o
TARGETS=libasn.so libasn.a

include rules.mk

pcre/.libs/libpcre.a:
	./pcre-build.sh

libasn.so: pcre/.libs/libpcre.a $(C_OBJECTS)
	$(CC) $(LDFLAGS) $(C_OBJECTS) -shared -o libasn.so pcre/.libs/libpcre.a

libasn.a: $(C_OBJECTS) pcre/.libs/libpcre.a
	$(AR) rc libasn.a $(C_OBJECTS) pcre/.libs/libpcre.a

install: all
	install -m 755 -d $(INSTALLPREFIX)/usr/local/include/libasn
	install -m 755 *.h $(INSTALLPREFIX)/usr/local/include/libasn
	install -m 755 -d $(INSTALLPREFIX)/usr/local/lib
	install -m 755 libasn.so $(INSTALLPREFIX)/usr/local/lib
