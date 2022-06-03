To compile the code, use:

g++ main.cpp cuboid.cpp framebuffer.cpp kdtree.cpp photonmap.cpp physics.cpp pinholecamera.cpp polymesh.cpp sphere.cpp -o main

And then run it using ./main

It will start photon mapping, which will take a while, before raytracing and printing its progress.

By default, the code will produce a 256x256 image with 24 depth of field samples. To get a higher quality image, change the height and width values in main.cpp and recompile.

To get an imaged produced faster, consider lowering the depth of field samples (sampleSize), lowering the image resolution or lowering the photon amounts.