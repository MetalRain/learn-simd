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
Run 0 | LINEAR: 0.122703s | THREADS: 0.207818s | SIMD: 0.097293s | SIMD + THREADS: 0.113367s
Run 1 | LINEAR: 0.117729s | THREADS: 0.202557s | SIMD: 0.096383s | SIMD + THREADS: 0.112284s
Run 2 | LINEAR: 0.118306s | THREADS: 0.210692s | SIMD: 0.096525s | SIMD + THREADS: 0.114158s
Run 3 | LINEAR: 0.118932s | THREADS: 0.201603s | SIMD: 0.098932s | SIMD + THREADS: 0.112472s
Run 4 | LINEAR: 0.118216s | THREADS: 0.220589s | SIMD: 0.098875s | SIMD + THREADS: 0.112886s
Run 5 | LINEAR: 0.117510s | THREADS: 0.204474s | SIMD: 0.097219s | SIMD + THREADS: 0.111936s
Run 6 | LINEAR: 0.119367s | THREADS: 0.198996s | SIMD: 0.098026s | SIMD + THREADS: 0.112253s
Run 7 | LINEAR: 0.116969s | THREADS: 0.204622s | SIMD: 0.100634s | SIMD + THREADS: 0.114153s
Run 8 | LINEAR: 0.119724s | THREADS: 0.199524s | SIMD: 0.096014s | SIMD + THREADS: 0.113209s
Run 9 | LINEAR: 0.119587s | THREADS: 0.196982s | SIMD: 0.099634s | SIMD + THREADS: 0.111442s
```
