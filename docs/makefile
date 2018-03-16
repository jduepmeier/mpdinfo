.PHONY: all clean install
PREFIX ?=/usr/local
MANPAGES=$(patsubst %.1,%.1.gz,$(wildcard *.1))

%.1.gz :: %.1
	gzip -c < $< > $@

all: $(MANPAGES)

install:
	install -g 0 -o 0 -m 0644 $(MANPAGES) $(DESTDIR)$(PREFIX)/share/man/man1/

clean:
	$(RM) $(MANPAGES)
