#
# ASN rules.mk, v. 1
#

CC ?= gcc
AR ?= ar

CFLAGS += -std=gnu99 -g -Wall -pedantic -fPIC -Dinline='inline __attribute__ ((gnu_inline))' $(CFLAGS_ADD)
LDFLAGS += -Wall -pedantic $(LDFLAGS_ADD)

default: all
all: $(TARGETS)

clean:
	-rm -f *.o $(TARGETS) *.core core

.SUFFIXES: .c

.c.o:
	$(CC) $(CFLAGS) -c $<

doc:
	-rm -fr doc
	mkdir -p doc
	doxygen doxygen.conf

.PHONY: doc
