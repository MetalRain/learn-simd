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

// Naive loop implementation
void k_means_linear(int point_count, int cluster_count, Point* points, Cluster* clusters) {
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
    assign_point(&(points[i]), &clusters[cluster_idx]);
  }
}

int k_means_simd(int point_count, int cluster_count, Point* points, Cluster* clusters) {
  int i = 0;
  const int points_per_loop = 4;
  const int processed_count = point_count - point_count % points_per_loop;

  // Find cluster for group of points
  while (i < processed_count) {

    // Load group of point coordinates as vectors
    __m128 xs = _mm_setr_ps(points[i].x, points[1+i].x, points[2+i].x, points[3+i].x);
    __m128 ys = _mm_setr_ps(points[i].y, points[1+i].y, points[2+i].y, points[3+i].y);

    __m128 smallests = _mm_set1_ps(FLT_MAX);
    __m128i labels = _mm_set1_epi32(0);
    int* label_ptr = (int*)&labels;

    for (int j=0; j < cluster_count; j++) {

      // Load cluster coordinates as vector
      __m128 cxs = _mm_set1_ps(clusters[j].x);
      __m128 cys = _mm_set1_ps(clusters[j].y);

      // subtract
      // dx = x - cx
      // dy = y - cy
      __m128 dxs = _mm_sub_ps(xs, cxs);
      __m128 dys = _mm_sub_ps(ys, cys);

      // multiply
      // dx = dx * dx
      // dy = dy * dy
      dxs = _mm_mul_ps(dxs, dxs);
      dys = _mm_mul_ps(dys, dys);

      // Distance to cluster (float)
      // [0.2, 0.1, 0.5, 0.6]
      __m128 distances = _mm_add_ps(dxs, dys);

      // Is new distance smaller than previous
      // [ -1, 0, -1, 0 ]
      __m128 results = _mm_cmplt_ps(distances, smallests);

      // If result is -1m use current cluster & set record smallest distance
      // Otherwise keep old value
      labels = _mm_setr_epi32(
        ((int)results[0]) == INT_MIN ? j : label_ptr[0],
        ((int)results[1]) == INT_MIN ? j : label_ptr[1],
        ((int)results[2]) == INT_MIN ? j : label_ptr[2],
        ((int)results[3]) == INT_MIN ? j : label_ptr[3]
      );
      smallests = _mm_setr_ps(
        ((int)results[0]) == INT_MIN ? distances[0] : smallests[0],
        ((int)results[1]) == INT_MIN ? distances[1] : smallests[1],
        ((int)results[2]) == INT_MIN ? distances[2] : smallests[2],
        ((int)results[3]) == INT_MIN ? distances[3] : smallests[3]
      );
    }

    // Add point to cluster
    for(int j=0; j < points_per_loop; j++) {
      assign_point(&(points[i + j]), &clusters[label_ptr[j]]);
    }

    i += points_per_loop;
  }

  return processed_count;
}


// Runs single iteration of k-means
bool k_means_it(int point_count, int cluster_count, Point* points, Cluster* clusters) {
  init_clusters(cluster_count, clusters);

  k_means_linear(point_count, cluster_count, points, clusters);

  return adjust_clusters(cluster_count, clusters);
}

// Runs single iteration of k-means
bool k_means_simd_it(int point_count, int cluster_count, Point* points, Cluster* clusters) {
  init_clusters(cluster_count, clusters);

  int processed_count = k_means_simd(point_count, cluster_count, points, clusters);
 
  // Process rest of points with linear
  if (processed_count < point_count) {

    Point* rem_ptr = points;
    rem_ptr += processed_count;
    k_means_linear(point_count - processed_count, cluster_count, rem_ptr, clusters);
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
        changed = k_means_simd_it(point_count, cluster_count, points, clusters);
        break;
      case k_means_linear_impl:
      default:
        changed = k_means_it(point_count, cluster_count, points, clusters);
        break;
    }

    // Loop until no change or max iterations has been reached
    if (!changed || iterations >= MAX_ITERATIONS){
      return iterations;
    }
  }

  return iterations;
}
