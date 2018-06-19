#include "vec3.h"

void vector_copy(Vec3* dest, Vec3* src){
  dest->x = src->x;
  dest->y = src->y;
  dest->z = src->z;
}

void vector_add(Vec3* dest, Vec3* v){
  dest->x += v->x;
  dest->y += v->y;
  dest->z += v->z;
}

void vector_sub(Vec3* dest, Vec3* v){
  dest->x -= v->x;
  dest->y -= v->y;
  dest->z -= v->z;
}

void vector_scale(Vec3* dest, float s){
  dest->x *= s;
  dest->y *= s;
  dest->z *= s;
}

void vector_mult(Vec3* dest, Vec3* v){
  dest->x *= v->x;
  dest->y *= v->y;
  dest->z *= v->z;
}

float vector_dot(Vec3* v1, Vec3* v2){
  return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
}

float vector_len2(Vec3* v){
  return vector_dot(v, v);
}

float vector_len(Vec3* v){
  return sqrtf(vector_len2(v));
}

void vector_normalize(Vec3* dest) {
  float len = vector_len(dest);
  if (len > 0.) {
    vector_scale(dest, 1. / len);
  }
}

void vector_scale_add(Vec3* dest, Vec3* v_scale, float mul, Vec3* v_add){
  vector_copy(dest, v_scale);
  vector_scale(dest, mul);
  vector_add(dest, v_add);
}
