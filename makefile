CFLAGS=-Wall -lmpdclient -lpthread -g
all: mpdinfo


mpdinfo: mpdinfo.c parse.c help.c format.c debug.c
