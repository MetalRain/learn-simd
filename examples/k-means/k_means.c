#include <stdio.h>
#include <string.h>
#include <float.h>
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