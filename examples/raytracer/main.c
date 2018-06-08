#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <float.h>

#define PI 3.141592653589793
#define TOO_FAR 10000000.

typedef struct vec3 {
  float x;
  float y;
  float z;
} Vec3;

typedef Vec3 Color;

typedef struct sphere {
  Vec3 center;
  float radius;
  float transparency;
  float reflection;
  Color surface;
  Color emission;
} Sphere;

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

Vec3 vector_mult(Vec3* v1, Vec3* v2){
  return (Vec3){
    v1->x * v2->x,
    v1->y * v2->y,
    v1->z * v2->z,
  };
}

float vector_dot(Vec3* v1, Vec3* v2){
  return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
}

float vector_len2(Vec3* v1){
  return vector_dot(v1, v1);
}

float vector_len(Vec3* v1){
  return sqrtf(vector_len2(v1));
}

Vec3 vector_normalize(Vec3* v) {
  float len = vector_len2(v);
  if (len > 0) {
    float inv_len = 1. / sqrtf(len);
    return vector_scale(v, inv_len);
  }
  return *v;
}

Vec3 vector_scale_add(Vec3* v, float a, Vec3* v2){
  Vec3 temp = vector_scale(v, a);
  return vector_add(&temp, v2);
}

// Ray inside sphere must go out as well
bool sphere_intersect(Vec3* from, Vec3* dir, Sphere* sphere, float* near, float* far){
  Vec3 direct = vector_sub(&sphere->center, from);
  float tca = vector_dot(&direct, dir);
  if (tca < 0) return false;
  float d2 = vector_dot(&direct, &direct) - tca * tca;
  if (d2 > sphere->radius * sphere->radius) return false;
  float thc = sqrtf(sphere->radius * sphere->radius - d2);
  *near = tca - thc;
  *far = tca + thc;
  return true;
}

void trace(Color* result, Vec3 from, Vec3 dir, const int obj_count, Sphere* objects, int rec_remaining) {

  Sphere* sphere = NULL;
  float surface_dist = TOO_FAR;

  for(int i=0; i < obj_count; i++){
    float near = TOO_FAR, far = TOO_FAR; 
    // Find nearest intersecting sphere
    if (sphere_intersect(&from, &dir, &objects[i], &near, &far)){
      if (near < 0) near = far;
      if (near < surface_dist) {
        surface_dist = near;
        sphere = &objects[i];
      }
    }
  }

  // No hits, return background color
  if (sphere == NULL) {
    result->x = 0.;
    result->y = 0.;
    result->z = 0.;
    return;
  }

  Color surface_color = {0.,0.,0.};
  Vec3 hit_point = vector_scale_add(&dir, surface_dist, &from);

  Vec3 hit_normal = vector_sub(&hit_point, &sphere->center);
  hit_normal = vector_normalize(&hit_normal);

  bool inside = false;
  const float bias = 0.01;

  if (vector_dot(&dir, &hit_normal) > 0){
    hit_normal = vector_scale(&hit_normal, -1.);
    inside = true;
  }

  if ((sphere->reflection > 0 || sphere->transparency) && rec_remaining > 0) {

    const float fresnel_mix = 0.1;
    float facing_ratio = -1. * vector_dot(&dir, &hit_normal);
    float fresnel_ratio = powf(1. - facing_ratio, 3.);
    float fresnel_effect = 1. * fresnel_mix + fresnel_ratio * (1. - fresnel_mix);

    // Reflection
    float mult = -2. * vector_dot(&dir, &hit_normal);
    Vec3 refl_dir = vector_scale_add(&hit_normal, mult, &dir);
    refl_dir = vector_normalize(&refl_dir);

    Vec3 refl_origin = vector_scale_add(&hit_normal, bias, &hit_point);
    Color reflection_color = {0.,0.,0.};
    trace(&reflection_color, refl_origin, refl_dir, obj_count, objects, rec_remaining - 1);

    surface_color = vector_scale(&reflection_color, fresnel_effect);

    // Refraction
    if (sphere->transparency > 0.){
      float ior = 1.1,
            eta = (inside) ? ior : 1. / ior;

      float cosi = -1. * vector_dot(&hit_normal, &dir);
      float k = 1. - eta * eta * (1. - cosi * cosi);

      Vec3 refr_dir = vector_scale(&dir, eta);
      refr_dir = vector_scale_add(&hit_normal, eta * cosi - sqrtf(k), &refr_dir);
      refr_dir = vector_normalize(&refr_dir);

      Vec3 refr_origin = vector_scale_add(&hit_normal, -bias, &hit_point);
      Color refraction_color = {0.,0.,0.};
      trace(&refraction_color, refr_origin, refr_dir, obj_count, objects, rec_remaining - 1);

      refraction_color = vector_mult(&refraction_color, &(sphere->surface));
      surface_color = vector_scale_add(&refraction_color, (1. - fresnel_effect) * sphere->transparency, &surface_color);
    }

  } else {

    // No more bounces, so  calculate lights hitting this point
    for(int i=0; i < obj_count; i++){
      // all emissive spheres are lights
      if (objects[i].emission.x > 0){

        float transmission = 2.;
        Vec3 light_dir = vector_sub(&objects[i].center, &hit_point);
        light_dir = vector_normalize(&light_dir);

        // check if light is obstructed
        for(int j=0; j < obj_count; j++){
          if(i != j){
            float near, far;
            Vec3 obstruct_origin = vector_scale_add(&hit_normal, bias, &hit_point);
            if (sphere_intersect(&obstruct_origin, &light_dir, &objects[j], &near, &far)){
              transmission = 0.;
              break;
            }
          }
        }

        float dir_mult = fmax(0., vector_dot(&hit_normal, &light_dir));

        Color color_add = vector_scale(&sphere->surface, transmission * dir_mult);
        color_add = vector_mult(&color_add, &(sphere[i].emission));

        surface_color = vector_add(&surface_color, &color_add);
      }
    }
  }

  Color newResult = vector_add(&surface_color, &sphere->emission);
  result->x = newResult.x;
  result->y = newResult.y;
  result->z = newResult.z;
  return;
};

void render(int obj_count, Sphere* objects, Camera* camera) {
  printf("P3\n");
  printf("%d %d\n", camera->width, camera->height);
  printf("255\n");

  float inv_w = 1. / (float)camera->width;
  float inv_h = 1. / (float)camera->height;
  float aspect = camera->width / (float)camera->height;
  float angle = tanf(PI * 0.5 * camera->fov / 180.);

  Color result = {0.,0.,0.};

  for(int y=0; y < camera->height; y++){
    for(int x=0; x < camera->width; x++){
      float dx = ( 2. * ((x + .5) * inv_w) - 1. ) * angle * aspect;
      float dy = (1. - 2. * ((y + .5) * inv_h)) * angle;
      Vec3 dir = {dx, dy, -1};
      dir = vector_normalize(&dir);
      result = (Color){0.,0.,0.};
      trace(&result, camera->position, dir, obj_count, objects, 5);
      // Clamp to 0-1 and multiply
      int r = fmax(0., fmin(1., result.x)) * 255;
      int g = fmax(0., fmin(1., result.y)) * 255;
      int b = fmax(0., fmin(1., result.z)) * 255;
      printf("%d %d %d ", r, g, b);
    }
    printf("\n");
  }
}


int main(int argc, char **argv) {

  Sphere objects[] = {
    (Sphere){
      (Vec3){0., -10004., -20.}, 
      10000., 
      .0, 
      .0, 
      (Color){.2,.2,.2},
      (Color){0.,0.,0.}
    },
    (Sphere){
      (Vec3){0.,0.,-20.},
      2.5,
      1.,
      0.7,
      (Color){1.,0.32,0.36},
      (Color){0.,0.,0.}
    },
    (Sphere){
      (Vec3){5.,-1.,-15.},
      2.,
      1.,
      0.0,
      (Color){.9,0.76,0.46},
      (Color){0.,0.,0.}
    },
    (Sphere){
      (Vec3){5.,0.,-25.},
      3.,
      1.,
      0.0,
      (Color){.65,0.77,0.97},
      (Color){0.,0.,0.}
    },
    (Sphere){
      (Vec3){-5.5,0.,-15.},
      3.,
      1.,
      0.0,
      (Color){.9,0.9,0.9},
      (Color){0.,0.,0.}
    },
    // Light
    (Sphere){
      (Vec3){0., 10., -30.},
      1.,
      0.,
      0.,
      (Color){0.,0.,0.},
      (Color){3.,3.,3.}
    }
  };

  Camera camera = {
    (Vec3){0, 0, 0},
    640,
    480,
    30.
  };

  render(sizeof(objects) / sizeof(Sphere), objects, &camera);

  return 0;
}
