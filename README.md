# wave-raytrace

This project is a demo phong and snell's law wave simulation. There are 3 surfaces:

1. An image of a complex polynomial, colors indicate the angle of the complex output.
2. A translucent and reflective wave
3. A light above the wave

Which are shaded with the following features:
* Vertex shader elevates the wave and animates it through time. It is based off two 3d sin functions with offset periods.
* Phong lighting on the wave gives specular highlights and shadows
* Snell's Law refracts light through the wave and colors the wave based on polynomial
* Underneath the wave, the Painter's Reflection is used to lookup what color the wave should be
* Refraction looks up if the light is directly in line with the camera, washing out the wave's color.
* The Multipath effect is visible on complex polynomial, based off the wave and light above. The water acts as a magnifying
glass and caustic lighting is rendered.

Not all features are available with the android version, due to limitations with OpenGL ES

Here are some screenshots of the desktop version

![Top View](https://kwylder.com/wave-raytrace/top.png)
![Refraction View](https://kwylder.com/wave-raytrace/middle.png)
![Image View](https://kwylder.com/wave-raytrace/bottom.png)

To build the desktop version (on mac), make sure you have GLFW and GLEW installed 
```
brew install glfw3
brew install glew
```

Then make the desktop project and run it

```
cd desktop
make
./a.out
```



