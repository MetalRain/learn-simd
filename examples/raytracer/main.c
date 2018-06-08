#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <float.h>

#define PI 3.141592653589793 

typedef struct vec3 {
  float x;
  float y;
  float z;
} Vec3;

typedef struct color {
  float r;
  float g;
  float b;
} Color;

typedef struct sphere {
  Vec3 center;
  float radius;
  Color surface;
} Sphere;

typedef struct point_light {
  Vec3 position;
  Color emission;
} Light;

typedef struct camera {
  Vec3 position;
  unsigned width;
  unsigned height;
  float fov;
} Camera;

Vec3 vector_add(Vec3* v1, Vec3* v2){
  return (Vec3){
    v1->x + v2->x,
    v1->y + v2->y,
    v1->z + v2->z,
  };
}

Vec3 vector_sub(Vec3* v1, Vec3* v2){
  return (Vec3){
    v1->x - v2->x,
    v1->y - v2->y,
    v1->z - v2->z,
  };
}

Vec3 vector_scale(Vec3* v1, float s){
  return (Vec3){
    v1->x * s,
    v1->y * s,
    v1->z * s,
  };
}

float vector_dot(Vec3* v1, Vec3* v2){
  return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
}

float vector_len(Vec3* v1){
  return vector_dot(v1, v1);
}

Vec3 vector_normalize(Vec3* v) {
  float len = vector_len(v);
  if (len > 0) {
    float inv_len = 1 / sqrtf(len);
    return vector_scale(v, inv_len);
  }
  return *v;
}

// Ray inside sphere must go out as well
bool sphere_intersect(Vec3* from, Vec3* dir, Sphere* sphere, float* near, float* far){
  Vec3 direct = vector_sub(&sphere->center, from);
  float tca = vector_dot(&direct, dir);
  if (tca < 0) return false;
  float d2 = vector_len(&direct) - tca * tca;
  if (d2 > sphere->radius * sphere->radius) return false;
  float thc = sqrtf(sphere->radius * sphere->radius - d2);
  *near = tca - thc;
  *far = tca + thc;
  return true;
}

Color trace(Vec3 from, Vec3 dir, int obj_count, int light_count, Sphere* objects, Light* lights, int rec_remaining) {

  Sphere* sphere = NULL;
  float surface_dist = FLT_MAX;

  for(int i=0; i < obj_count; i++){
    float near = FLT_MAX, far = FLT_MAX; 
    // Find nearest intersecting sphere
    if (sphere_intersect(&from, &dir, &objects[i], &near, &far)){
      if (near < 0) near = far;
      if (near < surface_dist) {
        surface_dist = near;
        sphere = &objects[i];
      }
    }
  }

  if (sphere != NULL) {
    return sphere->surface;
  }
  return (Color){0,0,0};
};

void render(int obj_count, int light_count, Sphere* objects, Light* lights, Camera* camera) {
  printf("P3\n");
  printf("%d %d\n", camera->width, camera->height);
  printf("255\n");

  float inv_w = 1. / (float)camera->width;
  float inv_h = 1. / (float)camera->height;
  float aspect = camera->width / (float)camera->height;
  float angle = tanf(PI * 0.5 * camera->fov / 180.);

  for(int y=0; y < camera->height; y++){
    for(int x=0; x < camera->width; x++){
      float dx = ( 2 * ((x + .5) * inv_w) - 1 ) * angle * aspect;
      float dy = (1 - 2 * ((y + .5) * inv_h)) * angle;
      Vec3 dir = {dx, dy, -1};
      dir = vector_normalize(&dir);
      Color result = trace(camera->position, dir, obj_count, light_count, objects, lights, 5);
      int r = result.r;
      int g = result.g;
      int b = result.b;
      printf("%d %d %d ", r, g, b);
    }
    printf("\n");
  }
}


int main(int argc, char **argv) {

  Sphere objects[] = {
    (Sphere){
      (Vec3){0,0,-20},
      4.,
      (Color){255,0,0}
    }
  };

  Light lights[] = {
    (Light){
      (Vec3){0, 20, -30},
      (Color){255, 255, 255}
    }
  };

  Camera camera = {
    (Vec3){0, 0, 0},
    640,
    480,
    30.
  };

  render(1, 1, objects, lights, &camera);

  return 0;
}
