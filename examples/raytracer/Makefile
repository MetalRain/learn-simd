CC=gcc
CFLAGS=-march=native -O3
DEPS=

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

raytracer: main.o
	$(CC) -o raytracer main.o $(CFLAGS)