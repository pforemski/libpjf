CFLAGS =
LDFLAGS = -lm

ME=libpjf
C_OBJECTS=lib.o sfork.o regex.o thash.o tlist.o xstr.o wstr.o mmatic.o tsort.o \
	fc.o select.o unitype.o json.o rfc822.o linux.o encode.o blowfish.o \
	mime.o utf8.o

ifeq (,$(NOFIFOS))
C_OBJECTS+=fifos.o
endif

TARGETS=libpjf.so libpjf.a

include rules.mk

libpjf.so: $(C_OBJECTS)
	$(CC) $(C_OBJECTS) -shared -o libpjf.so $(LDFLAGS)

libpjf.a: $(C_OBJECTS)
	$(AR) rc libpjf.a $(C_OBJECTS)

install: install-std

utilities:
	$(MAKE) -C utils

distclean: clean
	$(MAKE) -C distclean

doc:
	doxygen doxygen.conf
