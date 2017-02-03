.PHONY: clean install

DESTDIR=/
INSTALL_DIR=usr/local/bin
CFLAGS=-Wall -lmpdclient
all: mpdinfo

debug: CFLAGS += -g
debug: mpdinfo

mpdinfo: mpdinfo.c help.c format.c status.c libs/*.c

install:
	install -m 0755 -D mpdinfo $(DESTDIR)$(INSTALL_DIR)/mpdinfo

run:
	valgrind --leak-check=full ./mpdinfo -c sample.conf

clean:
	rm -f mpdinfo

pack:
	mkdir -p mpdinfo-1.1.0
	cp -r *.h *.c makefile libs _mpdinfo README.md sample.conf mpdinfo-1.1.0
	tar -czf packages/mpdinfo-1.1.0.tar.gz mpdinfo-1.1.0
	rm -rf mpdinfo-1.1.0
