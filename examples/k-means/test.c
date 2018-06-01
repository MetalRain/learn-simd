#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include "dataset.h"

bool fl_eq4(float a, float b, float c, float d){
  return a == b && b == c && c == d;
}

bool int_eq4(int a, int b, int c, int d){
  return a == b && b == c && c == d;
}

bool implementations_return_same(int point_count, int cluster_count, Point* points, Cluster* clusters) {
  bool passed = true;
  const int clusters_size = cluster_count * sizeof(Cluster);
  float* xs = malloc(point_count * sizeof(float));
  float* ys = malloc(point_count * sizeof(float));

  PointsArray points_soa = {xs, ys};
  // Convert points from Array of structures to structure of arrays
  // for easier alignment with SIMD instructions
  for(int i = 0; i < point_count; i++){
    xs[i] = points[i].x;
    ys[i] = points[i].y;
  }

  Cluster* runtime_values = malloc(clusters_size);
  Cluster* result_simd = malloc(clusters_size);
  Cluster* result_linear = malloc(clusters_size);
  Cluster* result_simd_soa = malloc(clusters_size);
  Cluster* result_linear_soa = malloc(clusters_size);

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

  // Re-initialize clusters
  memcpy(runtime_values, clusters, clusters_size);

  // Run with linear
  k_means_soa(point_count, cluster_count, &points_soa, runtime_values, k_means_linear_impl);
  memcpy(result_linear_soa, runtime_values, clusters_size);

  // Re-initialize clusters
  memcpy(runtime_values, clusters, clusters_size);

  // Run with vector instructions
  k_means_soa(point_count, cluster_count, &points_soa, runtime_values, k_means_simd_impl);
  memcpy(result_simd_soa, runtime_values, clusters_size);
 
  // Compare results
  for(int i=0; i < cluster_count; i++){
    Cluster c_simd = result_simd[i];
    Cluster c_lin = result_linear[i];
    Cluster c_simd_soa = result_simd_soa[i];
    Cluster c_lin_soa = result_linear_soa[i];

    bool matches = (
      fl_eq4(c_simd.x, c_lin.x, c_simd_soa.x, c_lin_soa.x) &&
      fl_eq4(c_simd.y, c_lin.y, c_simd_soa.y, c_lin_soa.y) &&
      int_eq4(c_simd.cardinality, c_lin.cardinality, c_simd_soa.cardinality, c_lin_soa.cardinality)
    );

    if (!matches){
      printf("Error: Results differ in cluster %d\n", i);
      print_cluster(&c_lin);
      print_cluster(&c_lin_soa);
      print_cluster(&c_simd);
      print_cluster(&c_simd_soa);
      passed = false;
    }
  }

  free(xs);
  free(ys);
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

  Point points[] = {
    (Point){ 100., 200. }, // Cluster 0
    (Point){ 700., 600. }, // Cluster 2
    (Point){ 400., 300. }, // Cluster 1
    (Point){   0., 100. }, // Cluster 0
    (Point){ 900., 700. }, // Cluster 2
    (Point){ 600., 400. }, // Cluster 1
    (Point){ -50., -10. }, // Cluster 0
    (Point){ 999., 600. }, // Cluster 2
    (Point){ 500., 500. }  // Cluster 1
  };
  int cluster_count = sizeof(clusters) / sizeof(Cluster),
      point_count = sizeof(points) / sizeof(Point);

  return implementations_return_same(point_count, cluster_count, points, clusters);
}

bool implementations_return_same_dynamic() {

  int cluster_count = 5, point_count = 1000000;

  srand(0);

  Cluster* clusters = malloc(cluster_count * sizeof(Cluster));
  Point* points = malloc(point_count * sizeof(Point));

  gen_points(point_count, cluster_count, points, clusters);

  bool passed = implementations_return_same(point_count, cluster_count, points, clusters);

  free(clusters);
  free(points);

  return passed;
}

int main() {
  if (!implementations_return_same_static()) return 1;
  if (!implementations_return_same_dynamic()) return 1;

  return 0;
}