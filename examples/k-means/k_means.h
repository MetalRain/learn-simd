typedef struct point {
  float x;
  float y;
} Point;

typedef struct cluster {
  float x;
  float y;
  int cardinality;
  double cum_x;
  double cum_y;
} Cluster;

void print_point(Point* point);

void print_cluster(Cluster* cluster);

short k_means(int point_count, int mean_count, Point* points, Cluster* means);