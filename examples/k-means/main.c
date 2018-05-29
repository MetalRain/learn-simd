#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "dataset.h"

int main() {
  int cluster_count = 4, point_count = 1000000;
  short iterations;

  Cluster* clusters = malloc(cluster_count * sizeof(Cluster));
  Point* points = malloc(point_count * sizeof(Point));

  gen_points(point_count, cluster_count, points, clusters);

  clock_t start = clock(), diff;
  iterations = k_means(point_count, cluster_count, points, clusters);
  diff = clock() - start;

  double calculations = iterations * cluster_count * point_count;
  double seconds = (diff * 1000. / CLOCKS_PER_SEC) / 1000.;
  double perPointNano = (diff * 1000000000. / CLOCKS_PER_SEC) / calculations;

  printf("Finished in %d iterations\n Took %gs\n Per point %fns\n", iterations, seconds, perPointNano);

  // Print results
  for (int i=0; i < cluster_count; i++) {
    print_cluster(&clusters[i]);
  }

  free(clusters);
  free(points);

  return 0;
}
