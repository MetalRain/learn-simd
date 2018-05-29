#include <stdlib.h>
#include "dataset.h"

int main() {
  int i;
  int cluster_count = 4;
  int point_count = 1000000;

  Cluster* clusters = malloc(cluster_count * sizeof(Cluster));
  Point* points = malloc(point_count * sizeof(Point));

  gen_points(point_count, cluster_count, points, clusters);

  for(i=0; i < cluster_count; i++) {
    print_cluster(&clusters[i]);
  }

  k_means(point_count, cluster_count, points, clusters);

  // Print results
  for (int i=0; i < cluster_count; i++) {
    print_cluster(&clusters[i]);
  }

  free(clusters);
  free(points);

  return 0;
}
