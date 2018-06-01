#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "dataset.h"

int main() {
  const int cluster_count = 4, 
      point_count = 1000000,
      executions = 10;

  clock_t start, diff;
  double seconds;
  double linear_results[executions];
  double simd_results[executions];
  double soa_linear_results[executions];
  double soa_simd_results[executions];

  srand(0);

  Cluster* clusters = malloc(cluster_count * sizeof(Cluster));
  Cluster* initial_values = malloc(cluster_count * sizeof(Cluster));
  Point* points = malloc(point_count * sizeof(Point));
  float* xs = malloc(point_count * sizeof(float));
  float* ys = malloc(point_count * sizeof(float));

  PointsArray points_soa = {xs, ys};

  gen_points(point_count, cluster_count, points, clusters);

  // Convert points from Array of structures to structure of arrays
  // for easier alignment with SIMD instructions
  for(int i = 0; i < point_count; i++){
    xs[i] = points[i].x;
    ys[i] = points[i].y;
  }

  // k-means changes clusters, store initial values here
  memcpy(initial_values, clusters, cluster_count * sizeof(Cluster));

  printf("Running all implementations %d times, please wait..\n", executions);

  for(int i=0; i < executions; i++) {

    // Re-initialize clusters
    memcpy(clusters, initial_values, cluster_count * sizeof(Cluster));

    start = clock();
    k_means(point_count, cluster_count, points, clusters, k_means_simd_impl);
    diff = clock() - start;
    seconds = (diff * 1000. / CLOCKS_PER_SEC) / 1000.;
    simd_results[i] = seconds;
  }

  for(int i=0; i < executions; i++) {

    // Re-initialize clusters
    memcpy(clusters, initial_values, cluster_count * sizeof(Cluster));

    start = clock();
    k_means(point_count, cluster_count, points, clusters, k_means_linear_impl);
    diff = clock() - start;
    seconds = (diff * 1000. / CLOCKS_PER_SEC) / 1000.;
    linear_results[i] = seconds;
  }

  for(int i=0; i < executions; i++) {

    // Re-initialize clusters
    memcpy(clusters, initial_values, cluster_count * sizeof(Cluster));

    start = clock();
    k_means_soa(point_count, cluster_count, &points_soa, clusters, k_means_linear_impl);
    diff = clock() - start;
    seconds = (diff * 1000. / CLOCKS_PER_SEC) / 1000.;
    soa_linear_results[i] = seconds;
  }

  for(int i=0; i < executions; i++) {

    // Re-initialize clusters
    memcpy(clusters, initial_values, cluster_count * sizeof(Cluster));

    start = clock();
    k_means_soa(point_count, cluster_count, &points_soa, clusters, k_means_simd_impl);
    diff = clock() - start;
    seconds = (diff * 1000. / CLOCKS_PER_SEC) / 1000.;
    soa_simd_results[i] = seconds;
  }


  for(int i=0; i < executions; i++) {
    printf(
      "Run %d | LINEAR AOS: %fs | LINEAR SOA: %fs | SIMD AOS: %fs | SIMD SOA: %fs\n",
      i, 
      linear_results[i], soa_linear_results[i], simd_results[i], soa_simd_results[i]
    );
  }

  free(initial_values);
  free(clusters);
  free(points);

  return 0;
}
