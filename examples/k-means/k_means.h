typedef struct points {
  float* xs;
  float* ys;
} PointData;

typedef struct cluster {
  float x;
  float y;
  int cardinality;
  double cum_x;
  double cum_y;
} Cluster;

typedef enum {
  k_means_linear_impl,
  k_means_simd_impl
} KMeansImpl;

void print_cluster(Cluster* cluster);

short k_means(int point_count, int mean_count, PointData* points, Cluster* means, KMeansImpl impl);

short k_means_threaded(int thread_count, int point_count, int mean_count, PointData* points, Cluster* means, KMeansImpl impl);