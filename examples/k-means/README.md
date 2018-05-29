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
Finished in 31 iterations
 Took 1.04444s
 Per point 8.422863ns
Cluster: (350,968) size: 224923
Cluster: (80,317) size: 239674
Cluster: (14,719) size: 270876
Cluster: (346,591) size: 264527
```
