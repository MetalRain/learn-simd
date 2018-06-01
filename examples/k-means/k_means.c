#include <stdio.h>
#include <float.h>
#include <limits.h>
#include <stdbool.h>
#include <immintrin.h>
#include "k_means.h"

#define MAX_ITERATIONS 100
#define EPSILON 1e-3

// Debug printing

void print_point(Point* point) {
  Point p = *point;
  printf("Point: (%g,%g)\n", p.x, p.y);
}

void print_cluster(Cluster* cluster) {
  Cluster c = *cluster;
  printf("Cluster: (%g,%g) size: %d\n", c.x, c.y, c.cardinality);
}


void print_mm128 (const char * str, __m128 v) {
  printf("%s: [ %g | %g | %g | %g ]\n", str, v[0], v[1], v[2], v[3]);
}


void print_mm128i (const char * str, __m128i v) {
  int* ptr = (int*)&v;
  printf("%s: [ %d | %d | %d | %d ]\n", str, ptr[0], ptr[1], ptr[2], ptr[3]);
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

void assign_point(Point* point, Cluster* cluster) {
  cluster->cardinality += 1;
  cluster->cum_x += point->x;
  cluster->cum_y += point->y;
}

// Linear AOS implementation
void k_means_linear_aos(int point_count, int cluster_count, Point* points, Cluster* clusters) {
  double d, smallest;
  int cluster_idx = 0;

  // Find cluster for each point
  for (int i=0; i < point_count; i++) {
    cluster_idx = 0;
    smallest = FLT_MAX;
    for (int j=0; j < cluster_count; j++) {
      float dx = points[i].x - clusters[j].x;
      float dy = points[i].y - clusters[j].y;
      d = dx * dx + dy * dy;

      if (d < smallest) {
        smallest = d;
        cluster_idx = j;
      }
    }
    // Add point to cluster
    clusters[cluster_idx].cardinality += 1;
    clusters[cluster_idx].cum_x += points[i].x;
    clusters[cluster_idx].cum_y += points[i].y;
  }
}


// Linear SOA implementation
void k_means_linear_soa(int point_count, int cluster_count, PointsArray* points, Cluster* clusters) {
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

// Vectorized AOS implementation
int k_means_simd_aos(int point_count, int cluster_count, Point* points, Cluster* clusters) {
  int i = 0;
  const int points_per_loop = 8;
  const int processed_count = point_count - point_count % points_per_loop;

  // Cluster centers don't change during iteration
  // load vectors at start
  __m256 cluster_x[cluster_count];
  __m256 cluster_y[cluster_count];

  for(int j=0; j < cluster_count; j++){
    cluster_x[j] = _mm256_set1_ps(clusters[j].x);
    cluster_y[j] = _mm256_set1_ps(clusters[j].y);
  }

  // Find cluster for group of points
  while (i < processed_count) {

    // Load group of point coordinates as vectors
    __m256 xs = _mm256_setr_ps(
      points[0+i].x, points[1+i].x, points[2+i].x, points[3+i].x,
      points[4+i].x, points[5+i].x, points[6+i].x, points[7+i].x
    );
    __m256 ys = _mm256_setr_ps(
      points[0+i].y, points[1+i].y, points[2+i].y, points[3+i].y,
      points[4+i].y, points[5+i].y, points[6+i].y, points[7+i].y
    );

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
      clusters[label_ptr[j]].cum_x += points[i + j].x;
      clusters[label_ptr[j]].cum_y += points[i + j].y;
    }

    i += points_per_loop;
  }

  return processed_count;
}


// Vectorized SOA implementation
int k_means_simd_soa(int point_count, int cluster_count, PointsArray* points, Cluster* clusters) {
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


// Runs single iteration of k-means
bool k_means_it_aos(int point_count, int cluster_count, Point* points, Cluster* clusters) {
  init_clusters(cluster_count, clusters);

  k_means_linear_aos(point_count, cluster_count, points, clusters);

  return adjust_clusters(cluster_count, clusters);
}

// Runs single iteration of k-means
bool k_means_it_soa(int point_count, int cluster_count, PointsArray* points, Cluster* clusters) {
  init_clusters(cluster_count, clusters);

  k_means_linear_soa(point_count, cluster_count, points, clusters);

  return adjust_clusters(cluster_count, clusters);
}


// Runs single iteration of k-means
bool k_means_simd_it_aos(int point_count, int cluster_count, Point* points, Cluster* clusters) {
  init_clusters(cluster_count, clusters);

  int processed_count = k_means_simd_aos(point_count, cluster_count, points, clusters);
 
  // Process rest of points with linear
  if (processed_count < point_count) {

    Point* rem_ptr = points;
    rem_ptr += processed_count;
    k_means_linear_aos(point_count - processed_count, cluster_count, rem_ptr, clusters);
  }

  return adjust_clusters(cluster_count, clusters);
}

// Runs single iteration of k-means
bool k_means_simd_it_soa(int point_count, int cluster_count, PointsArray* points, Cluster* clusters) {
  init_clusters(cluster_count, clusters);

  int processed_count = k_means_simd_soa(point_count, cluster_count, points, clusters);
 
  // Process rest of points with linear
  if (processed_count < point_count) {

    float* rem_x = points->xs;
    float* rem_y = points->ys;

    rem_x += processed_count;
    rem_y += processed_count;

    PointsArray remaining = {rem_x, rem_y};

    k_means_linear_soa(point_count - processed_count, cluster_count, &remaining, clusters);
  }

  return adjust_clusters(cluster_count, clusters);
}

short k_means(int point_count, int cluster_count, Point* points, Cluster* clusters, KMeansImpl impl) {
  short iterations = 0;

  while(true){
    iterations++;
    bool changed;

    switch(impl){
      case k_means_simd_impl:
        changed = k_means_simd_it_aos(point_count, cluster_count, points, clusters);
        break;
      case k_means_linear_impl:
      default:
        changed = k_means_it_aos(point_count, cluster_count, points, clusters);
        break;
    }

    // Loop until no change or max iterations has been reached
    if (!changed || iterations >= MAX_ITERATIONS){
      return iterations;
    }
  }

  return iterations;
}

short k_means_soa(int point_count, int cluster_count, PointsArray* points, Cluster* clusters, KMeansImpl impl) {
  short iterations = 0;

  while(true){
    iterations++;
    bool changed;

    switch(impl){
      case k_means_simd_impl:
        changed = k_means_simd_it_soa(point_count, cluster_count, points, clusters);
        break;
      case k_means_linear_impl:
      default:
        changed = k_means_it_soa(point_count, cluster_count, points, clusters);
        break;
    }

    // Loop until no change or max iterations has been reached
    if (!changed || iterations >= MAX_ITERATIONS){
      return iterations;
    }
  }

  return iterations;
}
