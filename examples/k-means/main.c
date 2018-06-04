#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "dataset.h"

double min(int count, double* results){
  double current = results[0];
  for(int i=0; i < count; i++){
    if (results[i] < current){
      current = results[i];
    }
  }
  return current;
}

double max(int count, double* results){
  double current = results[0];
  for(int i=0; i < count; i++){
    if (results[i] > current){
      current = results[i];
    }
  }
  return current;
}

double avg(int count, double* results){
  double sum;
  for(int i=0; i < count; i++){
    sum += results[i];
  }
  return sum / count;
}

int main() {
  const int cluster_count = 4, thread_count = 2;

  const int point_count = 1000000,
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

  printf("Running all implementations with %d points and %d clusters..\n", point_count, cluster_count);

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

  printf("LINEAR         - min: %gs, max: %gs, avg: %gs\n", min(executions, linear_results), max(executions, linear_results), avg(executions, linear_results));
  printf("THREADS        - min: %gs, max: %gs, avg: %gs\n", min(executions, threaded_results), max(executions, threaded_results), avg(executions, threaded_results));
  printf("SIMD           - min: %gs, max: %gs, avg: %gs\n", min(executions, simd_results), max(executions, simd_results), avg(executions, simd_results));
  printf("SIMD + THREADS - min: %gs, max: %gs, avg: %gs\n", min(executions, threaded_simd_results), max(executions, threaded_simd_results), avg(executions, threaded_simd_results));

  free(xs);
  free(ys);
  free(initial_values);
  free(clusters);

  return 0;
}
