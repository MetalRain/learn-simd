#include "k_means.h"

int main() {
  Point points[] = {
    (Point){1, 2},
    (Point){2, 3},
    (Point){6, 8},
    (Point){9, 7},
    (Point){9, 6},
    (Point){8, 1},
    (Point){4, 7},
    (Point){7, 3},
    (Point){8, 6},
    (Point){3, 1}
  };
  Cluster means[] = {
    (Cluster){1, 1, 0, 0, 0},
    (Cluster){5, 5, 0, 0, 0},
    (Cluster){8, 8, 0, 0, 0},
  };
  /*
  0123456789|
  1  X    X |
  2XO    O  |
  3 X    X  |
  4         |
  5         |
  6      0 X|
  7   X    X|
  8       X |
  9         |
  ----------+
  Legend:
  - point X
  - cluster O
  - point & cluster 0
  */

  int point_count = sizeof(points) / sizeof(Point);
  int mean_count = sizeof(means) / sizeof(Cluster);

  k_means(point_count, mean_count, points, means);

  // Print results
  for (int i=0; i < mean_count; i++) {
    print_cluster(&means[i]);
  }

  return 0;
}
