#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "dataset.h"

int main() {
  const int cluster_count = 4,
    thread_count = 4;

  const int point_count = 32000 * 4,
    executions = 10;

  clock_t start, diff;
  double seconds;
  double linear_results[executions];
  double simd_results[executions];
  double threaded_results[executions];
  double threaded_simd_results[executions];

  srand(0);

  Cluster* clusters = malloc(cluster_count * sizeof(Cluster));
  Cluster* initial_values = malloc(cluster_count * sizeof(Cluster));
  float* xs = malloc(point_count * sizeof(float));
  float* ys = malloc(point_count * sizeof(float));

  PointData points = {xs, ys};

  gen_points(point_count, cluster_count, &points, clusters);

  // k-means changes clusters, store initial values here
  memcpy(initial_values, clusters, cluster_count * sizeof(Cluster));

  printf("Running all implementations %d times, please wait..\n", executions);

  for(int i=0; i < executions; i++) {

    // Re-initialize clusters
    memcpy(clusters, initial_values, cluster_count * sizeof(Cluster));

    start = clock();
    k_means(point_count, cluster_count, &points, clusters, k_means_linear_impl);
    diff = clock() - start;
    seconds = (diff * 1000. / CLOCKS_PER_SEC) / 1000.;
    linear_results[i] = seconds;
  }

  for(int i=0; i < executions; i++) {

    // Re-initialize clusters
    memcpy(clusters, initial_values, cluster_count * sizeof(Cluster));

    start = clock();
    k_means(point_count, cluster_count, &points, clusters, k_means_simd_impl);
    diff = clock() - start;
    seconds = (diff * 1000. / CLOCKS_PER_SEC) / 1000.;
    simd_results[i] = seconds;
  }

   for(int i=0; i < executions; i++) {

    // Re-initialize clusters
    memcpy(clusters, initial_values, cluster_count * sizeof(Cluster));

    start = clock();
    k_means_threaded(thread_count, point_count, cluster_count, &points, clusters, k_means_linear_impl);
    diff = clock() - start;
    seconds = (diff * 1000. / CLOCKS_PER_SEC) / 1000.;
    threaded_results[i] = seconds;
  }

  for(int i=0; i < executions; i++) {

    // Re-initialize clusters
    memcpy(clusters, initial_values, cluster_count * sizeof(Cluster));

    start = clock();
    k_means_threaded(thread_count, point_count, cluster_count, &points, clusters, k_means_simd_impl);
    diff = clock() - start;
    seconds = (diff * 1000. / CLOCKS_PER_SEC) / 1000.;
    threaded_simd_results[i] = seconds;
  }


  for(int i=0; i < executions; i++) {
    printf(
      "Run %d | LINEAR: %fs | THREADS: %fs | SIMD: %fs | SIMD + THREADS: %fs\n",
      i, 
      linear_results[i], threaded_results[i], simd_results[i], threaded_simd_results[i]
    );
  }

  free(xs);
  free(ys);
  free(initial_values);
  free(clusters);

  return 0;
}
