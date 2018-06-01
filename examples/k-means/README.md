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
Run 0 | LINEAR AOS: 0.784599s | LINEAR SOA: 0.778465s | SIMD AOS: 0.687410s | SIMD SOA: 0.651464s
Run 1 | LINEAR AOS: 0.785229s | LINEAR SOA: 0.776055s | SIMD AOS: 0.688832s | SIMD SOA: 0.656960s
Run 2 | LINEAR AOS: 0.781252s | LINEAR SOA: 0.781518s | SIMD AOS: 0.694275s | SIMD SOA: 0.651307s
Run 3 | LINEAR AOS: 0.779119s | LINEAR SOA: 0.775116s | SIMD AOS: 0.695831s | SIMD SOA: 0.650199s
Run 4 | LINEAR AOS: 0.778955s | LINEAR SOA: 0.782768s | SIMD AOS: 0.692051s | SIMD SOA: 0.653055s
Run 5 | LINEAR AOS: 0.782575s | LINEAR SOA: 0.787393s | SIMD AOS: 0.694791s | SIMD SOA: 0.650325s
Run 6 | LINEAR AOS: 0.778044s | LINEAR SOA: 0.780889s | SIMD AOS: 0.691301s | SIMD SOA: 0.648727s
Run 7 | LINEAR AOS: 0.784683s | LINEAR SOA: 0.777229s | SIMD AOS: 0.686188s | SIMD SOA: 0.647249s
Run 8 | LINEAR AOS: 0.786584s | LINEAR SOA: 0.777491s | SIMD AOS: 0.689030s | SIMD SOA: 0.649455s
Run 9 | LINEAR AOS: 0.779005s | LINEAR SOA: 0.775659s | SIMD AOS: 0.686462s | SIMD SOA: 0.648623s
```
