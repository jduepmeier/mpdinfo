INSTALLPATH=/usr/local/bin
CFLAGS=-Wall -lmpdclient
.PHONY: clean install
all: mpdinfo

debug: CFLAGS += -g
debug: mpdinfo

mpdinfo: mpdinfo.c help.c format.c status.c libs/*.c

install:
	install -m 0755 mpdinfo $(INSTALLPATH)

run:
	valgrind --leak-check=full ./mpdinfo -c sample.conf

clean:
	rm mpdinfo
