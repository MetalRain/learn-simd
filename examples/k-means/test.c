#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include "dataset.h"

bool implementations_return_same(int point_count, int cluster_count, PointData* points, Cluster* clusters) {
  bool passed = true;
  const int clusters_size = cluster_count * sizeof(Cluster);

  Cluster* runtime_values = malloc(clusters_size);
  Cluster* result_simd = malloc(clusters_size);
  Cluster* result_linear = malloc(clusters_size);

  // Init clusters
  memcpy(runtime_values, clusters, clusters_size);

  // Run with linear
  k_means(point_count, cluster_count, points, runtime_values, k_means_linear_impl);
  memcpy(result_linear, runtime_values, clusters_size);

  // Re-initialize clusters
  memcpy(runtime_values, clusters, clusters_size);

  // Run with vector instructions
  k_means(point_count, cluster_count, points, runtime_values, k_means_simd_impl);
  memcpy(result_simd, runtime_values, clusters_size);

  // Compare results
  for(int i=0; i < cluster_count; i++){
    Cluster c_simd = result_simd[i];
    Cluster c_lin = result_linear[i];

    bool matches = (
      c_simd.x == c_lin.x &&
      c_simd.y == c_lin.y &&
      c_simd.cardinality == c_lin.cardinality
    );

    if (!matches){
      printf("Error: Results differ in cluster %d\n", i);
      print_cluster(&c_lin);
      print_cluster(&c_simd);
      passed = false;
    }
  }

  free(result_linear);
  free(result_simd);
  free(runtime_values);

  return passed;
}

bool implementations_return_same_static() {
  Cluster clusters[] = {
    (Cluster){ 250., 250., 0, 0, 0},
    (Cluster){ 500., 500., 0, 0, 0},
    (Cluster){ 750., 750., 0, 0, 0}
  };

  float xs[] = {100, 700, 400, 0,   900, 600, -50, 999, 500};
  float ys[] = {200, 600, 300, 100, 700, 400, -10, 600, 500};

  PointData points = {xs, ys};

  int cluster_count = sizeof(clusters) / sizeof(Cluster),
      point_count = sizeof(xs) / sizeof(float);

  return implementations_return_same(point_count, cluster_count, &points, clusters);
}

bool implementations_return_same_dynamic() {

  int cluster_count = 5, point_count = 1000000;

  srand(0);

  Cluster* clusters = malloc(cluster_count * sizeof(Cluster));
  float* xs = malloc(point_count * sizeof(float));
  float* ys = malloc(point_count * sizeof(float));

  PointData points = {xs, ys};

  gen_points(point_count, cluster_count, &points, clusters);

  bool passed = implementations_return_same(point_count, cluster_count, &points, clusters);

  free(clusters);
  free(xs);
  free(ys);

  return passed;
}

int main() {
  if (!implementations_return_same_static()) return 1;
  if (!implementations_return_same_dynamic()) return 1;

  return 0;
}