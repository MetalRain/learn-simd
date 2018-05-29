#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "dataset.h"

int main() {
  int cluster_count = 4, point_count = 1000000;
  clock_t start, diff;
  double seconds;

  srand(0);

  Cluster* clusters = malloc(cluster_count * sizeof(Cluster));
  Cluster* dirty_clusters = malloc(cluster_count * sizeof(Cluster));
  Point* points = malloc(point_count * sizeof(Point));

  gen_points(point_count, cluster_count, points, clusters);

  // k-means changes clusters, store initial values here
  memcpy(dirty_clusters, clusters, cluster_count * sizeof(Cluster));

  const int executions = 10;
  double results[executions];
  double simd_results[executions];

  printf("Running both implementations 10 times, please wait..\n");

  for(int i=0; i < executions; i++) {

    // Re-initialize clusters
    memcpy(clusters, dirty_clusters, cluster_count * sizeof(Cluster));

    start = clock();
    k_means_simd(point_count, cluster_count, points, clusters);
    diff = clock() - start;
    seconds = (diff * 1000. / CLOCKS_PER_SEC) / 1000.;
    simd_results[i] = seconds;
  }

  for(int i=0; i < executions; i++) {

    // Re-initialize clusters
    memcpy(clusters, dirty_clusters, cluster_count * sizeof(Cluster));

    start = clock();
    k_means(point_count, cluster_count, points, clusters);
    diff = clock() - start;
    seconds = (diff * 1000. / CLOCKS_PER_SEC) / 1000.;
    results[i] = seconds;
  }

  for(int i=0; i < executions; i++) {
    printf("Time to cluster %d points, run %d | SIMD: %fs | NORMAL: %fs\n", point_count, i, simd_results[i], results[i]);
  }

  free(dirty_clusters);
  free(clusters);
  free(points);

  return 0;
}
