CFLAGS = -g -I../ -lasn
TARGETS=unitype json

all: $(TARGETS)

unitype: unitype.c
	gcc $(CFLAGS) unitype.c -o unitype

json: json.c
	gcc $(CFLAGS) json.c -o json

.PHONY: clean
clean:
	-rm -f $(TARGETS) *.o
