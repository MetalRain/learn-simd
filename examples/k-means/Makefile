CC=gcc
CFLAGS=-msse -msse2 -msse3 -mavx -m64
DEPS = dataset.h k_means.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

k_means: main.o dataset.o k_means.o
	$(CC) -o k-means main.o dataset.o k_means.o $(CFLAGS)