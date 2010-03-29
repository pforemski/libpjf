CFLAGS =
LDFLAGS = -lm

ME=libasn
C_OBJECTS=lib.o sfork.o regex.o thash.o tlist.o xstr.o wstr.o mmatic.o tsort.o \
	fc.o select.o fcml.o unitype.o json.o rfc822.o linux.o encode.o

ifeq (,$(NOFIFOS))
C_OBJECTS+=fifos.o
endif

TARGETS=libasn.so libasn.a

include rules.mk

pcre/.libs/libpcre.a:
	./pcre-build.sh

libasn.so: pcre/.libs/libpcre.a $(C_OBJECTS)
	$(CC) $(C_OBJECTS) -shared -o libasn.so pcre/.libs/libpcre.a $(LDFLAGS)

libasn.a: $(C_OBJECTS) pcre/.libs/libpcre.a
	$(AR) rc libasn.a $(C_OBJECTS) pcre/.libs/libpcre.a

install: install-std

utilities:
	$(MAKE) -C utils

distclean: clean
	$(MAKE) -C pcre distclean
