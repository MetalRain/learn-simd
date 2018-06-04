CC=gcc
CFLAGS=-march=native -pthread
DEPS = dataset.h k_means.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

k_means: main.o dataset.o k_means.o
	$(CC) -o k-means main.o dataset.o k_means.o $(CFLAGS)

test: test.o dataset.o k_means.o
	$(CC) -o k-means-tests test.o dataset.o k_means.o $(CFLAGS)