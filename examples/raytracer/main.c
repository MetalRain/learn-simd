#include <stdio.h>

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

void render(int obj_count, int light_count, Sphere* objects, Light* lights, Camera* camera) {
  printf("P3\n");
  printf("%d %d\n", camera->width, camera->height);
  printf("255\n");
  for(int y=0; y < camera->height; y++){
    for(int x=0; x < camera->width; x++){

      int r = 255;
      int g = 0;
      int b = 0;
      printf("%d %d %d ", r, g, b);
    }
    printf("\n");
  }
}


int main(int argc, char **argv) {

  Sphere objects[] = {
    (Sphere){
      (Vec3){0,0,0},
      10.,
      (Color){1,0,0}
    }
  };

  Light lights[] = {
    (Light){
      (Vec3){0, 10000, -10},
      (Color){1, 1, 1}
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
