#include <math.h>
#include <immintrin.h>
#include <stdio.h>

typedef __m128 Vec3;

// Debugging
void vector_print(const char* prefix, Vec3 *v);

// dot product of v1 and v2
float vector_dot(Vec3* v1, Vec3* v2);

// normalize dest between 0 - 1
void vector_normalize(Vec3* dest);