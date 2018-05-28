#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "k_means.h"

double distance(int x, int y, int x2, int y2) {
  return pow(x - x2, 2.) + pow(y - y2, 2.);
}

void print_point(Point* point) {
  Point p = *point;
  printf("Point: (%d,%d)\n", p.x, p.y);
}

void print_cluster(Cluster* cluster) {
  Cluster c = *cluster;
  printf("Cluster: (%d,%d) size: %d\n", c.x, c.y, c.cardinality);
}

// Runs single iteration of k-means
void k_means_it(int point_count, int mean_count, Point* points, Cluster* means) {
  int i = 0, j = 0, cluster_idx = 0;
  double d, smallest;

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
      d = distance(points[i].x, points[i].y, means[j].x, means[j].y);
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
      means[i].x = means[i].cum_x / means[i].cardinality;
      means[i].y = means[i].cum_y / means[i].cardinality;
    }
  }
}

void k_means(int point_count, int mean_count, Point* points, Cluster* means) {
  printf("Clustering %d points to %d clusters\n", point_count, mean_count);

  // Loop until change is small
  k_means_it(point_count, mean_count, points, means);
  k_means_it(point_count, mean_count, points, means);
  k_means_it(point_count, mean_count, points, means);
  k_means_it(point_count, mean_count, points, means);
}