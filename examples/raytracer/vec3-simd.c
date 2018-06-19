#include "vec3-simd.h"

void print_vec(const char* prefix, Vec3 *v){
  float* ptr = (float*)v;
  printf("%s: [%g, %g, %g, %g]@%p\n", prefix, ptr[0], ptr[1], ptr[2], ptr[3], v);
}

void vector_copy(Vec3* dest, Vec3* src){
  *dest = _mm_loadu_ps((float*)src);
}

void vector_add(Vec3* dest, Vec3* v){
  *dest = _mm_add_ps(*dest, *v);
}

void vector_sub(Vec3* dest, Vec3* v){
  *dest = _mm_sub_ps(*dest, *v);
}

void vector_scale(Vec3* dest, float s){
  __m128 mult = _mm_set1_ps(s);
  *dest = _mm_mul_ps(*dest, mult);
}

void vector_mult(Vec3* dest, Vec3* v){
  *dest = _mm_mul_ps(*dest, *v);
}

float vector_dot(Vec3* v1, Vec3* v2){
  __m128 mult = _mm_mul_ps(*v1, *v2);
  return mult[3] + mult[2] + mult[1];
}

float vector_len2(Vec3* v){
  return vector_dot(v, v);
}

float vector_len(Vec3* v){
  return sqrtf(vector_len2(v));
}

void vector_normalize(Vec3* dest) {
  // TODO: try _mm_invsqrt_ps 
  float len = vector_len(dest);
  if (len > 0.) {
    vector_scale(dest, 1. / len);
  }
}

void vector_scale_add(Vec3* dest, Vec3* v_scale, float mul, Vec3* v_add){
  __m128 mult = _mm_set1_ps(mul);
  *dest = _mm_fmadd_ps(*v_scale, mult, *v_add);
}
