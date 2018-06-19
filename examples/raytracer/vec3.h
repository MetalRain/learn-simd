#include <math.h>

#define ZERO_VEC (Vec3){0., 0., 0.}
#define ONE_VEC (Vec3){1., 1., 1.}

typedef struct vec3 {
  float x;
  float y;
  float z;
} Vec3;

// dest = src
void vector_copy(Vec3* dest, Vec3* src);

// dest = dest + v
void vector_add(Vec3* dest, Vec3* v);

// dest = dest - v
void vector_sub(Vec3* dest, Vec3* v);

// dest = dest * s
void vector_scale(Vec3* dest, float s);

// multiply element wise, dest = dest * v
void vector_mult(Vec3* dest, Vec3* v);

// dot product of v1 and v2
float vector_dot(Vec3* v1, Vec3* v2);

// square of vector length
float vector_len2(Vec3* v);

// vector length
float vector_len(Vec3* v);

// normalize dest between 0 - 1
void vector_normalize(Vec3* dest);

// dest = v_scale * mul + v_add
void vector_scale_add(Vec3* dest, Vec3* v_scale, float mul, Vec3* v_add);
