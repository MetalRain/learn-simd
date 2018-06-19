#include "vec3-simd.h"

void vector_print(const char* prefix, Vec3 *v){
  float* ptr = (float*)v;
  printf("%s: [%g, %g, %g, %g]@%p\n", prefix, ptr[0], ptr[1], ptr[2], ptr[3], v);
}

float vector_dot(Vec3* v1, Vec3* v2){
  // TODO: try _mm_dp_ps
  __m128 mult = _mm_mul_ps(*v1, *v2);
  return mult[3] + mult[2] + mult[1];
}

void vector_normalize(Vec3* dest) {
  // TODO: try _mm_invsqrt_ps
  float len = sqrtf(vector_dot(dest, dest));
  if (len > 0.) {
    __m128 inv = _mm_set1_ps(1. / len);
    *dest = _mm_mul_ps(*dest, inv);
  }
}