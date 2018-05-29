typedef struct point {
  int x;
  int y;
} Point;

typedef struct cluster {
  int x;
  int y;
  int cardinality;
  long cum_x;
  long cum_y;
} Cluster;

void print_point(Point* point);

void print_cluster(Cluster* cluster);

short k_means(int point_count, int mean_count, Point* points, Cluster* means);