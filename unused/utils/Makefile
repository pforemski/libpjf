CFLAGS = -I../../
LDFLAGS = -lm -L../../

ME=libasn
TARGETS=libasn_example fcmldump fcmldump_static udpsend udprecv 822tojson jsonto822 \
	list-linux-interfaces

include ../../rules.mk

libasn_example: libasn_example.o
	$(CC) libasn_example.o -o libasn_example ../libasn.a ../pcre/.libs/libpcre.a $(LDFLAGS)

fcmldump: fcmldump.o
	$(CC) fcmldump.o -o fcmldump -L. -lasn $(LDFLAGS)

fcmldump_static: fcmldump.o
	$(CC) fcmldump.o -o fcmldump_static ../libasn.a $(LDFLAGS)

udpsend: udpsend.o
	$(CC) $^ -o $@ -lasn $(LDFLAGS)

udprecv: udprecv.o
	$(CC) $^ -o $@ -lasn $(LDFLAGS)

822tojson: 822tojson.o
	$(CC) $^ -o $@ -lasn $(LDFLAGS)

jsonto822: jsonto822.o
	$(CC) $^ -o $@ -lasn $(LDFLAGS)

list-linux-interfaces: list-linux-interfaces.o
	$(CC) $^ -o $@ -lasn $(LDFLAGS)

install: install-std

distclean: clean
