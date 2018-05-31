# K-means

This example has implementations of K-means algorithm with and without SIMD instructions.

## Requirements

CPU:
- x86-64
- AVX2
- FMA

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
Running both implementations 10 times, please wait..
Time to cluster 1000000 points, run 0 | SIMD: 0.677542s | NORMAL: 0.785118s
Time to cluster 1000000 points, run 1 | SIMD: 0.678254s | NORMAL: 0.792409s
Time to cluster 1000000 points, run 2 | SIMD: 0.672852s | NORMAL: 0.786130s
Time to cluster 1000000 points, run 3 | SIMD: 0.671313s | NORMAL: 0.788299s
Time to cluster 1000000 points, run 4 | SIMD: 0.680093s | NORMAL: 0.782715s
Time to cluster 1000000 points, run 5 | SIMD: 0.676765s | NORMAL: 0.781520s
Time to cluster 1000000 points, run 6 | SIMD: 0.680364s | NORMAL: 0.782323s
Time to cluster 1000000 points, run 7 | SIMD: 0.678516s | NORMAL: 0.789870s
Time to cluster 1000000 points, run 8 | SIMD: 0.674686s | NORMAL: 0.790801s
Time to cluster 1000000 points, run 9 | SIMD: 0.673777s | NORMAL: 0.804875s

```
