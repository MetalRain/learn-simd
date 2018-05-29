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
Result 0 - SIMD: 0.249464 NORMAL: 0.154102
Result 1 - SIMD: 0.250891 NORMAL: 0.153315
Result 2 - SIMD: 0.250936 NORMAL: 0.155818
Result 3 - SIMD: 0.248642 NORMAL: 0.154685
Result 4 - SIMD: 0.253430 NORMAL: 0.154275
Result 5 - SIMD: 0.250122 NORMAL: 0.154283
Result 6 - SIMD: 0.249316 NORMAL: 0.154116
Result 7 - SIMD: 0.249558 NORMAL: 0.153039
Result 8 - SIMD: 0.253940 NORMAL: 0.154188
Result 9 - SIMD: 0.252191 NORMAL: 0.154250
```
