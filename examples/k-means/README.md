# K-means

This example implements [K-means](https://en.wikipedia.org/wiki/K-means_clustering) algorithm with and without using vector instructions.


## Requirements

- [GCC 5.4.0](https://gcc.gnu.org/)
- [POSIX threads](https://en.wikipedia.org/wiki/POSIX_Threads)
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
Test with 9 static values in 3 clusters
Test with 1 clusters and 30 points
Test with 2 clusters and 100 points
Test with 3 clusters and 1000 points
Test with 4 clusters and 10000 points
Test with 5 clusters and 100000 points
Test with 7 clusters and 500000 points
Test with 10 clusters and 1000000 points
```

Test program returns extra output when there is an error.

```
Error: Results differ in cluster 0
Linear
Cluster: (308.69,939.58) size: 78011
Vectorized
Cluster: (308.687,939.591) size: 78007
Linear threaded
Cluster: (308.69,939.58) size: 78011
Vectorized threaded
Cluster: (308.687,939.591) size: 78007

Error: Results differ in cluster 2
Linear
Cluster: (849.846,973.569) size: 90090
Vectorized
Cluster: (849.845,973.567) size: 90091
Linear threaded
Cluster: (849.846,973.569) size: 90090
Vectorized threaded
Cluster: (849.845,973.567) size: 90091

Error: Results differ in cluster 4
Linear
Cluster: (543.471,589.257) size: 104168
Vectorized
Cluster: (543.462,589.26) size: 104171
Linear threaded
Cluster: (543.471,589.257) size: 104168
Vectorized threaded
Cluster: (543.462,589.26) size: 104171
```


## Running

```
$ make
gcc -c -o main.o main.c -march=native
gcc -c -o dataset.o dataset.c -march=native
gcc -c -o k_means.o k_means.c -march=native
gcc -o k-means main.o dataset.o k_means.o -march=native

$ ./k-means
Running 10 times using 1600000 points, 4 clusters and 4 threads..
LINEAR         - min: 0.24966s, max: 0.256215s, avg: 0.252501s
THREADS        - min: 0.430026s, max: 0.496472s, avg: 0.469554s
SIMD           - min: 0.202142s, max: 0.20864s, avg: 0.20518s
SIMD + THREADS - min: 0.207848s, max: 0.210977s, avg: 0.209556s
```

Amount of threads can be given as argument, while default is 4:

```
$ ./k-means 1
Running 10 times using 1600000 points, 4 clusters and 1 threads..
LINEAR         - min: 0.250851s, max: 0.266925s, avg: 0.2578s
THREADS        - min: 0.252972s, max: 0.259913s, avg: 0.256325s
SIMD           - min: 0.204332s, max: 0.209357s, avg: 0.205887s
SIMD + THREADS - min: 0.205638s, max: 0.215048s, avg: 0.209222s
```

```
$ ./k-means 3
Running 10 times using 1600000 points, 4 clusters and 3 threads..
LINEAR         - min: 0.24952s, max: 0.25615s, avg: 0.251663s
THREADS        - min: 0.281761s, max: 0.28571s, avg: 0.283548s
SIMD           - min: 0.202261s, max: 0.209889s, avg: 0.20574s
SIMD + THREADS - min: 0.208881s, max: 0.210818s, avg: 0.20983s
```

```
$ ./k-means 7
Running 10 times using 1600000 points, 4 clusters and 7 threads..
LINEAR         - min: 0.249915s, max: 0.26727s, avg: 0.261017s
THREADS        - min: 0.350279s, max: 0.407008s, avg: 0.383324s
SIMD           - min: 0.205717s, max: 0.218853s, avg: 0.210132s
SIMD + THREADS - min: 0.208534s, max: 0.221264s, avg: 0.213122s
```

## Results


### Vector instructions

Currently implemented 256-bit vector instructions operate 8 floats at same time, but all in all reduction in execution time is about 20%. While point to cluster distance calculation seems quite optimal, point assignment to clusters still is handled manually.

Vector instruction implementation benefited from change in point data memory structure from array of structures to structure of arrays so that coordinates could be loaded without gather instructions.


### Threads

Current multithreading implementation seems only slow things down. Given that implementation needs to sync after every loop maybe multithreading would work better with bigger problem sizes.


### Implementation complexity

Implementing vector instructions and threads made relatively simple implementation quite more complicated, both requiring change from simple loop to executing loop in chunks. 

All in all, I think in this case paying complexity for 20% performance gain is not justifiable. But it would be interesting to compare how compiler auto-vectorization would perform with little help.