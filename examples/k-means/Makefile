CC=gcc
CFLAGS='-lm'
DEPS = k_means.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

k_means: main.o k_means.o
	$(CC) -o k-means main.o k_means.o $(CFLAGS)