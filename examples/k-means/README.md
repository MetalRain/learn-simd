# K-means

This example has implementations of K-means algorithm with and without SIMD instructions.

## Running

```
$ make
$ time ./k-means

Cluster: (383,886) size: 0
Cluster: (102,422) size: 0
Cluster: (154,810) size: 0
Cluster: (204,566) size: 0
Clustering 1000000 points to 4 clusters
Cluster: (350,968) size: 224923
Cluster: (80,317) size: 239674
Cluster: (14,719) size: 270876
Cluster: (346,591) size: 264527

real	0m1.093s
user	0m1.059s
sys	0m0.008s

```
