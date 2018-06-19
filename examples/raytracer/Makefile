CC=gcc
CFLAGS=-march=native -lm -O3
DEPS=

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

raytracer: main.o vec3.o
	$(CC) -o raytracer main.o vec3.o $(CFLAGS)