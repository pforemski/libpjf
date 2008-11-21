#
# ASN rules.mk, v. 2
#

CC ?= gcc
AR ?= ar

CFLAGS += -std=gnu99 -g -Wall -pedantic -fPIC -Dinline='inline __attribute__ ((gnu_inline))' $(CFLAGS_ADD)
LDFLAGS += -Wall -pedantic $(LDFLAGS_ADD)

ifneq (,$(shell ls ~/local 2>/dev/null))
CFLAGS += -I$(shell echo ~)/local/include
LDFLAGS += -L$(shell echo ~)/local/lib
PCPATH = $(shell echo ~)/local/lib/pkgconfig
endif

ifneq (,$(CFLAGS_PKGS))
CFLAGS += $(shell PKG_CONFIG_PATH=$(PCPATH) pkg-config $(CFLAGS_PKGS) --cflags)
LDFLAGS += $(shell PKG_CONFIG_PATH=$(PCPATH) pkg-config $(CFLAGS_PKGS) --libs)
endif

# for make install
PREFIX ?= /usr
PKGDST = $(DESTDIR)$(PREFIX)

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
