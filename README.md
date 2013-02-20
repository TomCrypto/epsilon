epsilon
=======

Physically Based Spectral Renderer

Work in progress, stand by.

To Be Noted: this code works for me, but it in its very early testing stages and the architecture is still being worked out. I do not recommend using the code for now.

Remember you need an OpenCL 1.2 device to run this. Do not try with 1.1 or 1.0, because it _will not work_. If you have no such GPU implementation, you can always try it out on the (free) Intel CPU SDK, which certainly supports OpenCL 1.2.

Windows users: you will need PDCurses to run this (just as Linux users will need ncurses). No other libraries beyond that and OpenCL are needed.

NOTE: the raytracing part has been brought down for the moment, to facilitate refactoring. Stand by!
