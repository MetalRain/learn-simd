# Raytracer

## Running

```
$ make
$ ./raytracer > result.ppm
$ xdg-open result.ppm
```

![Result image](https://github.com/MetalRain/learn-simd/blob/master/examples/raytracer/result.png)

Run with SIMD

```
$ make raytracer-simd
$ ./raytracer-simd > result-simd.ppm
$ xdg-open result-simd.ppm
```

## Resources

- https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-ray-tracing original C++ source stored in [scratch-a-pixel](./scratch-a-pixel/raytracer.cpp) folder.
- https://en.wikipedia.org/wiki/Netpbm_format#PPM_example