#include <stdio.h>
#include <float.h>
#include <limits.h>
#include <stdbool.h>
#include <immintrin.h>
#include "k_means.h"

#define MAX_ITERATIONS 100
#define EPSILON 1e-3

void print_point(Point* point) {
  Point p = *point;
  printf("Point: (%g,%g)\n", p.x, p.y);
}

void print_cluster(Cluster* cluster) {
  Cluster c = *cluster;
  printf("Cluster: (%g,%g) size: %d\n", c.x, c.y, c.cardinality);
}

// Runs single iteration of k-means
bool k_means_it(int point_count, int mean_count, Point* points, Cluster* means) {
  double d, smallest;
  int i = 0, j = 0, cluster_idx = 0;
  float dx, dy;
  bool changed = false;

  // Initialize clusters
  for (i=0; i < mean_count; i++) {
    means[i].cardinality = 0;
    means[i].cum_x = 0;
    means[i].cum_y = 0;
  }

  // Find cluster for each point
  for (i=0; i < point_count; i++) {
    cluster_idx = 0;
    smallest = FLT_MAX;
    for (j=0; j < mean_count; j++) {
      dx = points[i].x - means[j].x;
      dy = points[i].y - means[j].y;
      d = dx * dx + dy * dy;

      if (d < smallest) {
        smallest = d;
        cluster_idx = j;
      }
    }
    // Add point to cluster
    means[cluster_idx].cardinality += 1;
    means[cluster_idx].cum_x += points[i].x;
    means[cluster_idx].cum_y += points[i].y;
  }

  // Recalculate cluster positions
  for (i=0; i < mean_count; i++) {
    if (means[i].cardinality > 0){
      dx = means[i].cum_x / means[i].cardinality;
      dy = means[i].cum_y / means[i].cardinality;

      if (abs(means[i].x - dx) > EPSILON || abs(means[i].y - dy) > EPSILON) {
        changed = true;
      }

      means[i].x = dx;
      means[i].y = dy;
    }
  }

  return changed;
}

void print_mm128 (const char * str, __m128 v) {
  printf("%s: [ %g | %g | %g | %g ]\n", str, v[0], v[1], v[2], v[3]);
}

int fst(long long int i) {
  return (int) i;
}

int snd(long long int i) {
  return (int) (i >> 32);
}


void print_mm128i (const char * str, __m128i v) {
  int* ptr = (int*)&v;
  printf("%s: [ %d | %d | %d | %d ]\n", str, ptr[0], ptr[1], ptr[2], ptr[3]);
}

// Runs single iteration of k-means
bool k_means_simd_it(int point_count, int mean_count, Point* points, Cluster* means) {
  int i = 0, j = 0;
  float dx, dy;
  bool changed = false;

  const int points_per_loop = 4;

  // Initialize clusters
  for (i=0; i < mean_count; i++) {
    means[i].cardinality = 0;
    means[i].cum_x = 0;
    means[i].cum_y = 0;
  }

  // Find cluster for group of points
  for (i=0; i < point_count; i += points_per_loop) {

    // Load group of point coordinates as vectors
    __m128 xs = _mm_setr_ps(points[i].x, points[1+i].x, points[2+i].x, points[3+i].x);
    __m128 ys = _mm_setr_ps(points[i].y, points[1+i].y, points[2+i].y, points[3+i].y);

    __m128 smallests = _mm_set1_ps(FLT_MAX);
    __m128i labels = _mm_set1_epi32(0);
    int* label_ptr = (int*)&labels;

    for (j=0; j < mean_count; j++) {

      // Load cluster coordinates as vector
      __m128 cxs = _mm_set1_ps(means[j].x);
      __m128 cys = _mm_set1_ps(means[j].y);

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
    for(j=0; j < points_per_loop; j++) {
      means[label_ptr[j]].cardinality += 1;
      means[label_ptr[j]].cum_x += points[i].x;
      means[label_ptr[j]].cum_y += points[i].y;
    }
  }
  //TODO: handle remaining points

  // Recalculate cluster positions
  for (i=0; i < mean_count; i++) {
    if (means[i].cardinality > 0){
      dx = means[i].cum_x / means[i].cardinality;
      dy = means[i].cum_y / means[i].cardinality;

      if (abs(means[i].x - dx) > EPSILON || abs(means[i].y - dy) > EPSILON) {
        changed = true;
      }

      means[i].x = dx;
      means[i].y = dy;
    }
  }

  return changed;
}


short k_means(int point_count, int mean_count, Point* points, Cluster* means) {
  short iterations = 0;

  // Loop until no change or max iterations has been reached
  while(true){
    iterations++;
    bool changed = k_means_it(point_count, mean_count, points, means);
    if (!changed || iterations >= MAX_ITERATIONS){
      return iterations;
    }
  }

  return iterations;
}

short k_means_simd(int point_count, int mean_count, Point* points, Cluster* means) {
  short iterations = 0;

  // Loop until no change or max iterations has been reached
  while(true){
    iterations++;
    bool changed = k_means_simd_it(point_count, mean_count, points, means);
    if (!changed || iterations >= MAX_ITERATIONS){
      return iterations;
    }
  }

  return iterations;
}