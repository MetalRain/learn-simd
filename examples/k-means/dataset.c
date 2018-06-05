#include <stdlib.h>
#include "dataset.h"

#define MIN_X -1000
#define MIN_Y -1000
#define MAX_X  1000
#define MAX_Y  1000
#define MAX_DELTA 300

int from_range(int start, int end) {
  int range = abs(end - start);
  return start + (rand() % range);
}

void gen_points(int point_count, int cluster_count, PointData* points, Cluster* clusters) {
  int points_remaining = point_count;
  int last_index = 0, index;
  int cx, cy;
  int variance = cluster_count - 1;

  // Create clusters
  for (int i=0; i < cluster_count; i++) {
    cx = from_range(MIN_X, MAX_X);
    cy = from_range(MIN_Y, MAX_Y);
    clusters[i] = (Cluster){cx, cy, 0, 0, 0};

    int clusters_remaining = cluster_count - i;
    int rough_share = points_remaining / clusters_remaining;
    int share = rough_share;

    // last cluster gets remaining points
    if (clusters_remaining > 1) {
      share = from_range(share - (share / variance), share + share / variance);
    }

    points_remaining -= share;

    // Generate points near cluster
    for(int j=0; j < share; j++){
      index = last_index + j;
      points->xs[index] = from_range(cx - MAX_DELTA, cx + MAX_DELTA);
      points->ys[index] = from_range(cy - MAX_DELTA, cy + MAX_DELTA);
    }
    last_index += share;
  }
}