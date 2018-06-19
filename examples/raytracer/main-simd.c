#include <stdbool.h>
#include <float.h>
#include "vec3-simd.h"

#define PI 3.141592653589793
#define TOO_FAR 100000000.0

typedef Vec3 Color;

typedef struct sphere {
  Vec3 center;
  float radius;
  Color surface;
  float reflection;
  Color emission;
  float transparency;
} Sphere;

typedef struct camera {
  Vec3 position;
  unsigned width;
  unsigned height;
  float fov;
} Camera;

// Ray inside sphere must go out as well
bool sphere_intersect(Vec3* from, Vec3* dir, Sphere* sphere, float* near, float* far){
  Vec3 direct = _mm_set1_ps(0.);
  vector_add(&direct, &sphere->center);
  vector_sub(&direct, from);
  float tca = vector_dot(&direct, dir);
  if (tca < 0.) return false;
  float d2 = vector_dot(&direct, &direct) - tca * tca;
  float r2 = sphere->radius * sphere->radius;
  float dd = r2 - d2;
  if (dd < 0.) return false;
  float thc = sqrtf(dd);
  *near = tca - thc;
  *far = tca + thc;
  return true;
}

float mix_values(float a, float b, float mix){
  return b * mix + a * (1. - mix);
}

void trace(Color* result, Vec3* from, Vec3* dir, const int obj_count, Sphere* objects, int rec_remaining) {
  Sphere* sphere = NULL;
  float surface_dist = TOO_FAR;

  for(int i=0; i < obj_count; ++i){
    float near = TOO_FAR, far = TOO_FAR; 
    // Find nearest intersecting sphere
    if (sphere_intersect(from, dir, &objects[i], &near, &far)){
      if (near < 0) near = far;
      if (near < surface_dist) {
        surface_dist = near;
        sphere = &objects[i];
      }
    }
  }

  // No hits, return background color
  if (sphere == NULL) {
    *result = _mm_set_ps(2.0, 2.0, 2.0, 0.0);
    return;
  }

  Color surface_color = _mm_set1_ps(0.);
  Vec3 hit_point = _mm_set1_ps(0.);
  Vec3 hit_normal = _mm_set1_ps(0.);

  // hit point at surface of sphere
  vector_scale_add(&hit_point, dir, surface_dist, from);

  // surface normal of hit point
  vector_add(&hit_normal, &hit_point);
  vector_sub(&hit_normal, &sphere->center);
  vector_normalize(&hit_normal);

  bool inside = false;
  bool has_reflection = sphere->reflection > 0.;
  bool has_transparency = sphere->transparency > 0.;
  bool is_diffuse = !has_reflection && !has_transparency;

  const float bias = 0.01;

  if (vector_dot(dir, &hit_normal) > 0.){
    vector_scale(&hit_normal, -1.);
    inside = true;
  }

  if (rec_remaining > 0 && !is_diffuse) {

    float normal_dir_dot = -vector_dot(dir, &hit_normal);
    float fresnel_mix = powf(1. - normal_dir_dot, 4.);
    float fresnel_effect = mix_values(fresnel_mix, 1., 0.1);

    // Reflection
    Vec3 refl_dir = _mm_set1_ps(0.);
    vector_scale_add(&refl_dir, &hit_normal, 2. * normal_dir_dot, dir);
    vector_normalize(&refl_dir);

    Vec3 refl_origin = _mm_set1_ps(0.);
    vector_scale_add(&refl_origin, &hit_normal, bias, &hit_point);
    Color reflection_color = _mm_set1_ps(0.);
    trace(&reflection_color, &refl_origin, &refl_dir, obj_count, objects, rec_remaining - 1);

    vector_scale(&reflection_color, fresnel_effect);
    vector_scale(&reflection_color, sphere->reflection);
    vector_add(&surface_color, &reflection_color);

    // Refraction
    if (has_transparency){
      float ior = 1.1,
            eta = (inside) ? ior : 1. / ior;

      float cosi = normal_dir_dot;
      float k = 1. - eta * eta * (1. - cosi * cosi);

      Vec3 refr_ray = _mm_set1_ps(0.);
      vector_copy(&refr_ray, dir);
      vector_scale(&refr_ray, eta);

      Vec3 refr_dir = _mm_set1_ps(0.);
      vector_scale_add(&refr_dir, &hit_normal, eta * cosi - sqrtf(k), &refr_ray);
      vector_normalize(&refr_dir);

      Vec3 refr_origin = _mm_set1_ps(0.);
      vector_scale_add(&refr_origin, &hit_normal, -bias, &hit_point);

      Color refraction_color = _mm_set1_ps(0.);
      trace(&refraction_color, &refr_origin, &refr_dir, obj_count, objects, rec_remaining - 1);

      vector_scale(&refraction_color, (1. - fresnel_effect) * sphere->transparency);
      vector_mult(&refraction_color, &sphere->surface);
      vector_add(&surface_color, &refraction_color);
    }


  } else {

    // No more bounces, so  calculate lights hitting this point
    for(int i=0; i < obj_count; ++i){
      // all emissive spheres are lights
      if (objects[i].emission[0] > 0.){

        Sphere* light = &objects[i];
        Color transmission = _mm_set1_ps(1.);
        Vec3 light_dir;
        vector_copy(&light_dir, &light->center);
        vector_sub(&light_dir, &hit_point);

        // check if light is obstructed by another sphere
        for(int j=0; j < obj_count; ++j){
          if(i != j){
            float near, far;
            Vec3 obstruct_origin = _mm_set1_ps(0.);
            vector_scale_add(&obstruct_origin, &hit_normal, bias, &hit_point);
            if (sphere_intersect(&obstruct_origin, &light_dir, &objects[j], &near, &far)){
              transmission = _mm_set1_ps(0.);
              break;
            }
          }
        }

        Vec3 color_add = _mm_set1_ps(0.);
        vector_add(&color_add, &sphere->surface);
        vector_mult(&color_add, &transmission);
        vector_scale(&color_add, fmaxf(0., vector_dot(&hit_normal, &light_dir)));
        vector_mult(&color_add, &light->emission);
        vector_add(&surface_color, &color_add);
      }
    }
  }

  vector_add(result, &surface_color);
  vector_add(result, &sphere->emission);
};

void render(int obj_count, Sphere* objects, Camera* camera) {
  printf("P3\n");
  printf("%d %d\n", camera->width, camera->height);
  printf("255\n");

  float inv_w = 1. / (float)camera->width;
  float inv_h = 1. / (float)camera->height;
  float aspect = camera->width / (float)camera->height;
  float angle = tanf(PI * 0.5 * camera->fov / 180.);

  __m128 zero = _mm_set1_ps(0.);
  __m128 one = _mm_set1_ps(1.);
  __m128 max = _mm_set1_ps(255.);

  for(int y=0; y < camera->height; ++y){
    for(int x=0; x < camera->width; ++x){
      float dx = (2. * ((x + .5) * inv_w) - 1.) * angle * aspect;
      float dy = (1. - 2. * ((y + .5) * inv_h)) * angle;
      Vec3 dir = _mm_set_ps(dx, dy, -1., 0.);
      vector_normalize(&dir);
      Color result = _mm_set1_ps(0.);
      trace(&result, &camera->position, &dir, obj_count, objects, 5);

      // Clamp to 0-1 and multiply
      result = _mm_min_ps(result, one);
      result = _mm_max_ps(result, zero);
      result = _mm_mul_ps(result, max);
      int r = result[3];
      int g = result[2];
      int b = result[1];
      printf("%d %d %d ", r, g, b);
    }
    printf("\n");
  }
}


int main(int argc, char **argv) {

  Sphere objects[] = {
    (Sphere){
      _mm_set_ps(0., -10004., -20., 0.), 
      10000.,
      _mm_set_ps(0.4, 0.4, 0.4, 0.),
      0.4,
      _mm_set1_ps(0.),
      0.9
    },
    // Red
    (Sphere){
      _mm_set_ps(0.0, 0.0, -30.0, 0.),
      4.,
      _mm_set_ps(1., 0.32, 0.36, 0.),
      1.,
      _mm_set1_ps(0.),
      0.6
    },
    // Yellow
    (Sphere){
      _mm_set_ps(4., -2.0, -10., 0.),
      2.,
      _mm_set_ps(0.9, 0.76, 0.46, 0.),
      1.,
      _mm_set1_ps(0.),
      0.8
    },
    // Blue
    (Sphere){
      _mm_set_ps(5., 0., -34., 0.),
      3.,
      _mm_set_ps(.65, 0.77, 0.97, 0.),
      1.,
      _mm_set1_ps(0.),
      0.5
    },
    // Gray
    (Sphere){
      _mm_set_ps(-6., 2.5, -25., 0.),
      3.,
      _mm_set_ps(0.9, 0.9, 0.9, 0.0),
      1.,
      _mm_set1_ps(0.),
      0.5
    },
    // Light
    (Sphere){
      _mm_set_ps(40., 100., 50., 0.),
      15.,
      _mm_set_ps(1.0, 1.0, 1.0, 0.0),
      0.,
      _mm_set1_ps(30.),
      1.0
    }
  };

  Camera camera = {
    _mm_set1_ps(0.),
    640,
    480,
    45.
  };

  int obj_count = sizeof(objects) / sizeof(Sphere);

  render(obj_count, objects, &camera);

  return 0;
}
