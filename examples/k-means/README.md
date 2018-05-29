# K-means

This example has implementations of K-means algorithm with and without SIMD instructions.

## Running

```
$ make
gcc -c -o main.o main.c '-lm'
gcc -c -o dataset.o dataset.c '-lm'
gcc -c -o k_means.o k_means.c '-lm'
gcc -o k-means main.o dataset.o k_means.o '-lm'

$ ./k-means
Running without SIMD
Finished in 22 iterations
 Took 0.789313s
 Per point 8.969466ns
Cluster: (353.423,968.233) size: 223512
Cluster: (78.1031,319.193) size: 240917
Cluster: (15.6478,723.624) size: 270973
Cluster: (346.098,589.445) size: 264598

Running with SIMD
Finished in 5 iterations
 Took 0.216106s
 Per point 10.805300ns
Cluster: (290.761,847.627) size: 308685
Cluster: (123.888,444.025) size: 312592
Cluster: (129.23,668.949) size: 205962
Cluster: (223.622,621.472) size: 172761

```
