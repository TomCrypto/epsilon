epsilon
=======

Physically Based Spectral Renderer

Work in progress, stand by.

To Be Noted: this code works for me, but it in its very early testing stages and the architecture is still being worked out. I do not recommend using the code for now.

Remember you need an OpenCL 1.2 device to run this. Do not try with 1.1 or 1.0, because it _will not work_. If you have no such GPU implementation, you can always try it out on the (free) Intel CPU SDK, which certainly supports OpenCL 1.2.

Dependencies:
- ncurses (PDCurses for Windows)
- OpenCL (a 1.2 device is needed)
- Google Protobuf suite (not *really* yet, but it may be used soon)

NOTE: the raytracing part is back up, now with CPU/GPU BVH acceleration! though you'll find the performance kind of sucks on GPU's, given it's a direct port and still needs to be optimized for GPU memory patterns.
