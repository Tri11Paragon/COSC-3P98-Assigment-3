# Introduction
- Jacob wanted to see it.
- freeglut is required. 
	- install with `sudo apt install freeglut3-dev`
- build with:
	- `git clone --recursive https://github.com/Tri11Paragon/COSC-3P98-Assigment-3.git`
	- `cd COSC-3P98-Assigment-3/`
	- `mkdir build && cd build/`
	- `cmake -DCMAKE_BUILD_TYPE=Release ../`
	- `make -j 16`
	- `./assign3`
- Windows supported (MSVC only), Linux is preferred.
## Description
On my hardware, the simple particle fountain can reach 30k
particles and features sorted transparency.

### Extra Features

-   Ordered Alpha Blending

-   \"Spray\" Mode (12)

-   Textured Particles/Plane/Cube (16)

-   Particles with Different Textures (18)

-   Extra Feature - \"Performance Mode\" (23)

### Missing Features

The random spin mode was left out intentionally for two reasons. One, I
specifically designed the particle structure to fit in 32 bytes, half
the width of a cache line. Two, the potential marks were not worth
disturbing the particle data structure and further altering the particle
system. There is likely little benefit to ensuring the particles fit
nicely inside a cache line as most of the CPU time is spent on OpenGL
API calls. See \"Performance Mode\" for more information.

## Building
### Caveats

The assignment makes use of a non-standard OpenGL extension during
texture loading. \"GL_TEXTURE_MAX_ANISOTROPY_EXT\" should work on all
modern Intel/AMD/Nvidia hardware, if it doesn't work on your hardware
consider removing the line from texture.h and high_perf.cpp

### Build Commands

        mkdir build && cd build
        cmake -DCMAKE_BUILD_TYPE=Release ../
        make -j 16
        ./assign3

## Usage

Keybindings and usage instructions are printed at program startup.

---

---

# Performance Mode

## Design

The high-performance mode is the result of a weekend hack-a-ton where I
wanted to see how easy it would be to implement a million particle+
renderer. If I had more time I would encapsulate the high_perf.cpp file
into the particle system class, allowing for multiple \*and
customizable\* particle systems. If you wish to change settings, most
are constants in shaders/physics.comp or high_perf/high_perf.cpp. The
rendering engine itself can handle around 20 million particles at about
60fps. With physics enabled, the engine can handle about 6 million particles but the renderer is fillrate limited. Solutions to increase the number of rendered particles are
discussed in the future plans section. The new renderer makes use of \"GL_POINTS\" with a geometry shader to generate the vertices and features billboarding/texturing. A
compute shader is used before rendering to update the particle positions
and directions on the GPU. This way there is no need to copy the data to
and from the graphics card.

## Renderer

The legacy OpenGL renderer uses display lists to speed up the rendering
of particles. Although this method is faster than using the same draw
commands inline, it is highly limited by driver overhead. Modern GPUs
are designed to process massive amounts of data all at once and benefit
from reducing the amount of synchronization between the GPU and CPU. As
mentioned earlier the current renderer uses a vertex buffer object to
store all particle positions and directions in one giant array. It then
uses those points to render all particles in a single draw call, thereby
reducing driver overhead.

### Rendering Pipeline

#### Vertex Shader

The vertex shader is purely used to pass through the particle position
to the geometry shader. Since the vertex shader cannot output multiple
vertices (not easily at least), we have to use a geometry shader.

#### Geometry Shader

The geometry shader uses the up and right vectors from the inverse view
matrix to generate a quad facing the camera. It takes in the particle
position and outputs a triangle strip. This is a highly efficient
operation as there is dedicated hardware to handle this particular
geometry shader case (1:4)[@amdprogram p. 9].

#### Fragment Shader

The fragment shader is run once per pixel and is responsible for
texturing the particles. I use a texture array as it can be bound once
before rendering, therefore particles do not need to be separated by
texture. Using an array has the downside of every texture needing to be
the same size, to solve this I resize the texture as it is loaded.
Unfortunately, this will lead to some textures being distorted but the
performance gain is worth it. The modern renderer is constrained by the
lack of 'advanced' programming techniques, some of which are discussed
in the future plans section.

## Compute Shader

Compute shaders are very useful for embarrassingly parallel tasks like
updating particles. The compute shader is a very simple (nearly 1:1)
translation of the CPU version of the particle system's update function.
It handles 'dead' particles by resetting them to the initial
position/direction. As a result, particles are initialized with a random
lifetime between 0 and the max lifetime to ensure even distribution. If
you change the particle lifetime, please modify both constants!

#### Direction Offseting

Because generating random numbers on the GPU is hard (there is no
dedicated hardware random number generator), I generate a random set of
offsets at startup and upload these randoms to the GPU. The particle
index is then used to access this buffer when the particle is reset; the
result is a convincing distribution of particles. The large the number
of particles the larger the offset buffer should be. Up to 6 million
8192 should be fine. If things look off consider increasing the value to
some larger power of two. Make sure you update both constants here as
well!

## Usage

### Building

Add \"-DEXTRAS=ON\" to the CMake command.

            mkdir build && cd build
            cmake -DCMAKE_BUILD_TYPE=Release -DEXTRAS=ON ../
            make -j 16
            ./assign3

### Running

All particles exist from the beginning, which means all particles start
at the initial position and slowly spread out. After starting the
program but before moving around, you should press 'p' to allow the
compute shader to run, once the particles spread out it is safe to move.
The slow performance of all the particles in the same spot has to do
with overdraw accessing and writing the same location of the depth
texture (hard to do in parallel). Fillrate is a common issue with this
particle renderer. See the future plans section for possible
resolutions.

## Future Plans

Unfortunately, because this is exam season, I do not have time to do
anything more with this assignment. I plan to return to this project in the
future, and below is a list of features I began looking into.

### Lists

I would like to make it so all particles are not rendered all the time.
add a dead/alive particle list. This would prevent the issue of all
particles starting in the same place, the low performance that causes,
and would be helpful in sorting.

### Bitonic Sort

Professor Robson spent a good deal of time on this algorithm in the
parallel computing class and most of the literature online suggests
using this, including the famous GDC talk on optimized particle
systems. The next step in implementing a good
GPU-accelerated particle system is bitonic sorting, however as I got
further into the algorithm it became clear that if I wanted to implement
it myself, properly understanding the algorithm would take too much
time.

### Tiling

Tiling is a rendering technique that works by dividing the view frustum
into small sections and then sorting / culling particles within. This
way we can reduce the overall rendering load while staying on the GPU.
This is an algorithm I've always wanted to implement but lack a solid
understanding of. The GDC talk goes further into this and many online
resources talk about light clustering. Again, if I had
more time I would've learned it for this assignment as I think that
would have been cool.

### Hierarchical Depth Buffers

The idea is that by generating mipmaps of the depth buffer we can do
broad phase culling of particles, thereby reducing the number of
fine-grained (per pixel) accesses to the depth buffer. This is a
micro-optimization as stated by Mike Turitzin, but \"every ms
counts\".

### Lighting

Since tiling particles comes from Forward+ rendering, it would make
sense to implement a forward+ renderer.

## Figures

![20 million particles distributed in a 50x25x50 cube with load
monitors](screenshot002.png)

![20 million particles on the new
renderer](screenshot003.png)

![6.4 million particles, fillrate (not compute)
limited.](screenshot004.png)

![6.4 million particles, zoomed out, showing fillrate as the limiting
factor in speed.](screenshot005.png)

# References
- [OpenGL Compute Shaders](https://web.engr.oregonstate.edu/~mjb/cs519/Handouts/compute.shader.2pp.pdf)
- [OpenGL Reference Manual](https://registry.khronos.org/OpenGL-Refpages/gl4/)
- [Compute-Based GPU Particle Systems](https://ubm-twvideo01.s3.amazonaws.com/o1/vault/GDC2014/Presentations/Gareth_Thomas_Compute-based_GPU_Particle.pdf)
- [Particle Billboarding with the Geometry Shader](https://www.geeks3d.com/20140815/particle-billboarding-with-the-geometry-shader-glsl/)
- [ATI Radeon HD 2000 programming guide](https://web.archive.org/web/20160722164341/http://amd-dev.wpengine.netdna-cdn.com/wordpress/media/2012/10/ATI_Radeon_HD_2000_programming_guide.pdf)
- [Hierarchical Depth Buffers](https://miketuritzin.com/post/hierarchical-depth-buffers/)
