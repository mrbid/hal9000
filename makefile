all:
	gcc hal9000.c -Ofast -lSDL2 -lm -o hal9000

install:
	cp hal9000 $(DESTDIR)

uninstall:
	rm $(DESTDIR)/hal9000

clean:
	rm hal9000
