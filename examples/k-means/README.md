# K-means

This example has implementations of K-means algorithm with and without SIMD instructions.

## Running

```
$ make
gcc -c -o main.o main.c -march=native
gcc -c -o dataset.o dataset.c -march=native
gcc -c -o k_means.o k_means.c -march=native
gcc -o k-means main.o dataset.o k_means.o -march=native

$ ./k-means
Running both implementations 10 times, please wait..
Time to cluster 1000000 points, run 0 | SIMD: 0.735190s | NORMAL: 0.796717s
Time to cluster 1000000 points, run 1 | SIMD: 0.755416s | NORMAL: 0.793284s
Time to cluster 1000000 points, run 2 | SIMD: 0.746245s | NORMAL: 0.799630s
Time to cluster 1000000 points, run 3 | SIMD: 0.748949s | NORMAL: 0.988517s
Time to cluster 1000000 points, run 4 | SIMD: 0.734046s | NORMAL: 0.944742s
Time to cluster 1000000 points, run 5 | SIMD: 0.747311s | NORMAL: 0.792709s
Time to cluster 1000000 points, run 6 | SIMD: 0.730126s | NORMAL: 0.791240s
Time to cluster 1000000 points, run 7 | SIMD: 0.733360s | NORMAL: 0.788729s
Time to cluster 1000000 points, run 8 | SIMD: 0.735455s | NORMAL: 0.787744s
Time to cluster 1000000 points, run 9 | SIMD: 0.757140s | NORMAL: 0.786902s
```
