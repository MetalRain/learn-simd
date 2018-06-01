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
Running all implementations 10 times, please wait..
Run 0 | LINEAR: 0.821147s | SIMD: 0.683453s
Run 1 | LINEAR: 0.822704s | SIMD: 0.691980s
Run 2 | LINEAR: 0.855390s | SIMD: 0.694981s
Run 3 | LINEAR: 0.839654s | SIMD: 0.691994s
Run 4 | LINEAR: 0.832535s | SIMD: 0.694071s
Run 5 | LINEAR: 0.818484s | SIMD: 0.688475s
Run 6 | LINEAR: 0.821095s | SIMD: 0.706415s
Run 7 | LINEAR: 0.818277s | SIMD: 0.696803s
Run 8 | LINEAR: 0.829243s | SIMD: 0.683764s
Run 9 | LINEAR: 0.835043s | SIMD: 0.701297s
```
