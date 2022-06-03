# C++ Raycast Renderer

This was made as a coursework project in my third year at University of Bath. Small sections of some files were provided by the lecturer to all students - these files begin with a comment with "krt - Kens Raytracer".
The program renders a scene, as defined within main.cpp, to a 2D image. It includes the following features:

-  Local lighting calculations with the Phong reflection model
-  Support for other BRDFs in general
-  Raytracing trees for full specular (mirror-like) reflection and refraction
-  Photon mapping for:
   - Global illumination (diffuse interreflection)
   - Caustics
   - Use of shadow photons for efficiency
- Depth of field effect by supersampling rays

These effects can all be seen in the example image, FinalImage.png.

## Usage

To compile the code, use:

g++ main.cpp cuboid.cpp framebuffer.cpp kdtree.cpp photonmap.cpp physics.cpp pinholecamera.cpp polymesh.cpp sphere.cpp -o main

And then run it using ./main

It will start photon mapping, which will take a while, before raytracing and printing its progress.

By default, the code will produce a 256x256 image with 24 depth of field samples. To get a higher quality image, change the height and width values in main.cpp and recompile.

To get an imaged produced faster, consider lowering the depth of field samples (sampleSize), lowering the image resolution or lowering the photon amounts.
