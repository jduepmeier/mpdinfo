INSTALLPATH=/usr/local/bin
CFLAGS=-Wall -lmpdclient -lpthread
.PHONY: all debug install
all: mpdinfo

debug: CFLAGS += -g
debug: mpdinfo

mpdinfo: mpdinfo.c parse.c help.c format.c debug.c status.c

install:
	install -m 0755 mpdinfo $(INSTALLPATH)
clean:
	rm mpdinfo
