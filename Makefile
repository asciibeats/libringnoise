CFLAGS = -O3 -finline-functions -fPIC -fmax-errors=1 -Isrc -Wall -Werror
LDLIBS = -lm

INCLUDEDIR := /usr/include
LIBDIR := /usr/lib

SOURCES=$(wildcard src/*.c)
OBJECTS=$(patsubst %.c, %.o, $(SOURCES))
OUTFILE=libringnoise.so

$(OUTFILE): $(OBJECTS)
	gcc -shared -o $(OUTFILE) $(OBJECTS) $(LDLIBS)

$(OBJECTS): src/%.o: src/%.c
	gcc $(CFLAGS) -c $< -o $@

install:
	cp src/ringnoise.h $(INCLUDEDIR)
	cp $(OUTFILE) $(LIBDIR)

uninstall:
	rm -f $(INCLUDEDIR)/ringnoise.h
	rm -f $(LIBDIR)/$(OUTFILE)

clean:
	rm -f $(OUTFILE) src/*.o
