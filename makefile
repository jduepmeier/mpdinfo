.PHONY: clean install

PREFIX?=/usr/local
CFLAGS+=-Wall -std=gnu99
LDFLAGS+=-lmpdclient

OBJECTS= $(patsubst %.c, %.o, $(wildcard *.c libs/*.c))
DEPS= $(wildcard *.h libs/*.h)

all: mpdinfo

debug: CFLAGS += -g
debug: mpdinfo

mpdinfo: $(OBJECTS)

%.o: %.c $(DEPS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -c -o $@ $<
	$(MAKE) -C docs


install:
	install -m 0755 -D mpdinfo $(DESTDIR)$(PREFIX)/bin/mpdinfo
	$(MAKE) -C docs install

run:
	valgrind --leak-check=full ./mpdinfo -c sample.conf

clean:
	$(RM) $(OBJECTS)
	$(RM) mpdinfo
	$(MAKE) -C docs clean

pack:
	mkdir -p mpdinfo-1.1.0
	cp -r *.h *.c makefile libs _mpdinfo README.md sample.conf mpdinfo-1.1.0
	tar -czf packages/mpdinfo-1.1.0.tar.gz mpdinfo-1.1.0
	rm -rf mpdinfo-1.1.0
