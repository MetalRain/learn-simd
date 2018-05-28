# K-means

This example has implementations of K-means algorithm with and without SIMD instructions.

## Running

```
$ gcc -o k-means k-means.c -lm
$ time ./k-means

Point count: 10
Cluster: (5,4) size: 10
Cluster: (9,4) size: 0

real	0m0.001s
user	0m0.000s
sys	0m0.001s
```
