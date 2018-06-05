#include <stdio.h>
#include <float.h>
#include <limits.h>
#include <stdbool.h>
#include <immintrin.h>
#include <pthread.h>
#include <string.h>
#include "k_means.h"

#define MAX_ITERATIONS 100
#define EPSILON 1e-3

// Debug printing
void print_cluster(Cluster* cluster) {
  Cluster c = *cluster;
  printf("Cluster: (%g,%g) size: %d\n", c.x, c.y, c.cardinality);
}

// Preparation for each iteration
void init_clusters(int cluster_count, Cluster* clusters) {
  for (int i=0; i < cluster_count; i++) {
    clusters[i].cardinality = 0;
    clusters[i].cum_x = 0;
    clusters[i].cum_y = 0;
  }
}

// Accumulate cluster positions
bool adjust_clusters(int cluster_count, Cluster* clusters) {
  float dx, dy;
  bool changed = false;

  for (int i=0; i < cluster_count; i++) {
    if (clusters[i].cardinality > 0){
      dx = clusters[i].cum_x / clusters[i].cardinality;
      dy = clusters[i].cum_y / clusters[i].cardinality;

      if (abs(clusters[i].x - dx) > EPSILON || abs(clusters[i].y - dy) > EPSILON) {
        changed = true;
      }

      clusters[i].x = dx;
      clusters[i].y = dy;
    }
  }

  return changed;
}

// Linear implementation
void k_means_linear(int point_count, int cluster_count, PointData* points, Cluster* clusters) {
  double d, smallest;
  int cluster_idx = 0;
  float* xs = points->xs;
  float* ys = points->ys;

  // Find cluster for each point
  for (int i=0; i < point_count; i++) {
    cluster_idx = 0;
    smallest = FLT_MAX;
    for (int j=0; j < cluster_count; j++) {
      float dx = xs[i] - clusters[j].x;
      float dy = ys[i] - clusters[j].y;
      d = dx * dx + dy * dy;

      if (d < smallest) {
        smallest = d;
        cluster_idx = j;
      }
    }
    clusters[cluster_idx].cardinality += 1;
    clusters[cluster_idx].cum_x += xs[i];
    clusters[cluster_idx].cum_y += ys[i];
  }
}

// Vectorized implementation
int k_means_simd(int point_count, int cluster_count, PointData* points, Cluster* clusters) {
  int i = 0;
  const int points_per_loop = 8;
  const int processed_count = point_count - point_count % points_per_loop;
  const float* all_xs = points->xs;
  const float* all_ys = points->ys;

  // Cluster centers don't change during iteration
  // load vectors at start
  __m256 cluster_x[cluster_count];
  __m256 cluster_y[cluster_count];

  for(int j=0; j < cluster_count; j++){
    cluster_x[j] = _mm256_set1_ps(clusters[j].x);
    cluster_y[j] = _mm256_set1_ps(clusters[j].y);
  }

  __m256 xs;
  __m256 ys;

  // Find cluster for group of points
  while (i < processed_count) {

    const float* x_ptr = all_xs + i;
    const float* y_ptr = all_ys + i;

    // Load group of point coordinates as vectors
    xs = _mm256_loadu_ps(x_ptr);
    ys = _mm256_loadu_ps(y_ptr);

    __m256 smallests = _mm256_set1_ps(FLT_MAX);
    __m256i labels = _mm256_set1_epi32(0);
    int* label_ptr = (int*)&labels;

    for (int j=0; j < cluster_count; j++) {
      // subtract cluster center from point
      // dx = x - cx
      __m256 dxs = _mm256_sub_ps(xs, cluster_x[j]);
      // dy = y - cy
      __m256 dys = _mm256_sub_ps(ys, cluster_y[j]);

      // multiply
      // dx = dx * dx
      dxs = _mm256_mul_ps(dxs, dxs);

      // multiply & add
      // distance = dy * dy + dx 
      // Squared distance to cluster (float)
      // [0.2, 0.1, 0.5, 0.6]
      __m256 distances =_mm256_fmadd_ps(dys, dys, dxs);

      // resolve smallest values
      smallests = _mm256_min_ps(distances, smallests);

      // If smallest value is same as distance
      // assign current cluster as label
      labels = _mm256_setr_epi32(
        (smallests[0] == distances[0]) ? j : label_ptr[0],
        (smallests[1] == distances[1]) ? j : label_ptr[1],
        (smallests[2] == distances[2]) ? j : label_ptr[2],
        (smallests[3] == distances[3]) ? j : label_ptr[3],
        (smallests[4] == distances[4]) ? j : label_ptr[4],
        (smallests[5] == distances[5]) ? j : label_ptr[5],
        (smallests[6] == distances[6]) ? j : label_ptr[6],
        (smallests[7] == distances[7]) ? j : label_ptr[7]
      );
    }

    // Add points to assigned clusters
    for(int j=0; j < points_per_loop; j++) {
      clusters[label_ptr[j]].cardinality += 1;
      clusters[label_ptr[j]].cum_x += x_ptr[j];
      clusters[label_ptr[j]].cum_y += y_ptr[j];
    }

    i += points_per_loop;
  }

  return processed_count;
}

// Vectorized implementation, full iteration
void k_means_simd_run(int point_count, int cluster_count, PointData* points, Cluster* clusters) {
  int processed_count = k_means_simd(point_count, cluster_count, points, clusters);
 
  // Process rest of points with linear
  if (processed_count < point_count) {
    float* rem_x = points->xs;
    float* rem_y = points->ys;

    rem_x += processed_count;
    rem_y += processed_count;

    PointData remaining = {rem_x, rem_y};

    k_means_linear(point_count - processed_count, cluster_count, &remaining, clusters);
  }
}

typedef struct k_means_thread {
  int point_count;
  int cluster_count;
  PointData* points;
  Cluster* clusters;
} KMeansThread;


void* k_means_threaded_linear(void* ptr) {
  KMeansThread* thread_data = (KMeansThread*)ptr;
  k_means_linear(
    thread_data->point_count,
    thread_data->cluster_count,
    thread_data->points,
    thread_data->clusters
  );
  return NULL;
}

void* k_means_threaded_simd(void* ptr) {
  KMeansThread* thread_data = (KMeansThread*)ptr;
  k_means_simd_run(
    thread_data->point_count,
    thread_data->cluster_count,
    thread_data->points,
    thread_data->clusters
  );
  return NULL;
}

// Multithreaded linear implementation, full iteration
void k_means_threaded_run(int cluster_count, Cluster* clusters, void* fn (void *), 
  int thread_count, KMeansThread* thread_data, Cluster* thread_clusters, PointData* thread_points) {

  pthread_t thread[thread_count];

  
  for(int i=0; i < thread_count; i++){
    void* thread_data_ptr = (void*)&thread_data[i];

    // Map
    pthread_create(&(thread[i]), NULL, fn, thread_data_ptr);
  }

  // Reduce
  for(int i=0; i < thread_count; i++){
    pthread_join(thread[i], NULL);

    for(int j=0; j < cluster_count; j++){
      Cluster* thread_cluster_ptr = thread_clusters + i * cluster_count + j;
      clusters[j].cum_x += thread_cluster_ptr->cum_x;
      clusters[j].cum_y += thread_cluster_ptr->cum_y;
      clusters[j].cardinality += thread_cluster_ptr->cardinality;
    }
  }
}

short k_means_threaded(int thread_count, int point_count, int cluster_count, PointData* points, Cluster* clusters, KMeansImpl impl) {
  short iterations = 0;
  int clusters_size = cluster_count * sizeof(Cluster);

  KMeansThread* thread_data = malloc(thread_count * sizeof(KMeansThread));
  Cluster* thread_clusters = malloc(thread_count * clusters_size);
  PointData* thread_points = malloc(thread_count * sizeof(PointData));

  // Assign points to threads
  // and make copies of clusters
  int remaining_points = point_count;
  for(int i=0; i < thread_count; i++){

    int processed_count = point_count - remaining_points;

    int share = remaining_points / (thread_count - i);
    if (i == (thread_count - 1)) {
      share = remaining_points;
    }
    remaining_points -= share;

    Cluster* thread_cluster_ptr = thread_clusters + (i * cluster_count);

    float* thread_x = points->xs + processed_count;
    float* thread_y = points->ys + processed_count;

    thread_points[i] = (PointData){thread_x, thread_y};

    thread_data[i] = (KMeansThread){
      share,
      cluster_count,
      &thread_points[i],
      thread_cluster_ptr
    };
  }

  while(true){
    iterations++;
    bool changed;

    init_clusters(cluster_count, clusters);

    for(int i=0; i < thread_count; i++){
      Cluster* thread_cluster_ptr = thread_clusters + (i * cluster_count);
      // Init thread clusters
      memcpy(thread_cluster_ptr, clusters, clusters_size);
    }

    switch(impl){
      case k_means_simd_impl:
        k_means_threaded_run(cluster_count, clusters, k_means_threaded_simd,
          thread_count, thread_data, thread_clusters, thread_points);
        break;
      case k_means_linear_impl:
      default:
        k_means_threaded_run(cluster_count, clusters, k_means_threaded_linear,
          thread_count, thread_data, thread_clusters, thread_points);
        break;
    }

    changed = adjust_clusters(cluster_count, clusters);

    // Loop until no change or max iterations has been reached
    if (!changed || iterations >= MAX_ITERATIONS){
      break;
    }
  }

  // Clean up
  free(thread_clusters);
  free(thread_points);
  free(thread_data);

  return iterations;
}


short k_means(int point_count, int cluster_count, PointData* points, Cluster* clusters, KMeansImpl impl) {
  short iterations = 0;

  while(true){
    iterations++;
    bool changed;

    init_clusters(cluster_count, clusters);

    switch(impl){
      case k_means_simd_impl:
        k_means_simd_run(point_count, cluster_count, points, clusters);
        break;
      case k_means_linear_impl:
      default:
        k_means_linear(point_count, cluster_count, points, clusters);
        break;
    }

    changed = adjust_clusters(cluster_count, clusters);

    // Loop until no change or max iterations has been reached
    if (!changed || iterations >= MAX_ITERATIONS){
      break;
    }
  }

  return iterations;
}