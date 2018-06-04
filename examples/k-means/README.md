# K-means

This example implements [K-means](https://en.wikipedia.org/wiki/K-means_clustering) algorithm with and without using vector instructions.


## Requirements

- [GCC](https://gcc.gnu.org/)
- x86 instruction set extensions:
  - [AVX2](https://en.wikipedia.org/wiki/Advanced_Vector_Extensions#Advanced_Vector_Extensions_2)
  - [FMA3](https://en.wikipedia.org/wiki/FMA_instruction_set#FMA3_instruction_set)


## Test

```
$ make test
gcc -c -o test.o test.c -march=native
gcc -c -o dataset.o dataset.c -march=native
gcc -c -o k_means.o k_means.c -march=native
gcc -o k-means-tests test.o dataset.o k_means.o -march=native

./k-means-test
```

Test program returns output when there is an error.

```
Error: Results differ in cluster 0
Cluster: (250,250) size: 0
Cluster: (16.6667,96.6667) size: 3
Error: Results differ in cluster 1
Cluster: (500,500) size: 1
Cluster: (500,400) size: 3
Error: Results differ in cluster 2
Cluster: (456.125,361.25) size: 8
Cluster: (866.333,633.333) size: 3
```


## Running

```
$ make
gcc -c -o main.o main.c -march=native
gcc -c -o dataset.o dataset.c -march=native
gcc -c -o k_means.o k_means.c -march=native
gcc -o k-means main.o dataset.o k_means.o -march=native

$ ./k-means
Running all implementations with 1000000 points and 4 clusters..
LINEAR         - min: 0.838027s, max: 0.886473s, avg: 0.855094s
THREADS        - min: 0.83803s, max: 0.885312s, avg: 0.847477s
SIMD           - min: 0.704361s, max: 0.733262s, avg: 0.71796s
SIMD + THREADS - min: 0.704187s, max: 0.73941s, avg: 0.715165s
```
