εpsilon - Physically Based Spectral Renderer
============================================

This is a small, and self-contained, raytracing renderer I'm working on. It was
never designed to rival with existing high-profile renderers, as I developed it
mostly for fun and as an exercise in software design, though I am releasing it,
in the hope that someone will find it of some value.

The εpsilon renderer is coded in C++11, and is powered by OpenCL.

Work is currently being put into the following:

- Material system (only diffuse for testing at the moment)

- Optimizations (BVH traversal is unoptimized)

- Scene editor (this is not a priority)

- Documentation (mostly cpp/cl files)

Features
--------

- Intrinsically GPU-accelerated, as it was designed with OpenCL in mind.

- Simple, yet effective, renderer software architecture.

- Modular rendering back-end (component-based).

- Support for arbitrarily complex scenes.

- Nice console user interface.

- Uses XML for scene data.

- Doxygen documentation.

- Concise error log.

Usage
-----

First, build the renderer with your favorite C++11 compiler. Make sure it runs,
and that you actually have an OpenCL device, otherwise it will not do anything.

Run the renderer, and you will be presented with a console-based UI. Select the
OpenCL platform which contains the device you want to use - you probably have a
single platform - and then select the device. This can be either a CPU, GPU, or
other OpenCL-compliant hardware (if you happen to have any).

Then, type in the directory of the scene you wish to render. The directory must
not end in a slash, and a few test scenes are provided, in the `scenes` folder.
Make sure you decompress the scenes first, as they are distributed zipped (some
of them are quite large).

Now, just enter the file you want the renderer to output the final render in. A
PPM image will be produced there, which you can view and convert to PNG, etc...

Next, choose the desired width and height you wish to render the scene at. Note
that a large resolution will obviously take longer to render, a good resolution
is 512 × 512. 400 × 400 is nice too, and will take a bit less time.

Finally, choose the quantity of render passes the renderer should perform. This
is because the rendering algorithm used is probabilistic, and every render pass
is cumulative. Essentially, more passes will cause the final render to converge
to the physically correct image. More is better, but slower. A picture is worth
a thousand words:

<p align="center">
<img
src="https://raw.github.com/TomCrypto/epsilon/master/extra/pass.png"
alt="Render pass comparison"/>
</p>

Of course, the ideal number of passes highly depends on the scene, and notably,
the lighting conditions. Usually, it is best to start low, and increment slowly
making any necessary tweaks to the scene as you see more and more detail. After
you are satisfied with the result, set a high resolution, and number of passes,
and let it run.

Supported Platforms
-------------------

__LINUX__: The code is essentially native to Linux, and will run with little to
           no changes. Verified under Linux Mint 14 x64 and Debian Testing x64.

__WINDOWS__: The renderer is compatible with the latest MinGW distribution, and
             possibly MSVC. Verified under MinGW32 (GCC 4.7.2) / Windows 7 x86.
             It may be necessary to tweak the code a little bit if features are
             missing, but nothing too hard, at least for MinGW. We're making it
             better, one #ifdef at a time.

__MAC__: Status unknown - the renderer has never been tested under Mac OS X. If
         you want to help out with Mac support, drop me a line (contact details
         are on my Github page), all help is greatly welcome.

Dependencies
------------

- ncurses (PDCurses for Windows)
- A C++11 compliant compiler
- OpenCL

You will probably want an OpenCL 1.2 device, I haven't tried with an OpenCL 1.1
device but it will likely not work (either now or in the future) as I intend to
use OpenCL 1.2 features, such as image arrays to store spectral material data.

Note that all Intel CPU runtimes support OpenCL 1.2 and most GPU runtimes do as
well, so this should not be a problem. Make sure you have the headers, too.

Build Options
-------------

The following special build flags can be passed to alter some sections of code,
either for testing or compatibility - simply override the makefile, as follows:

    CXXFLAGS = [previous flags] -Dflag # where flag is the build flag

And then rebuild all. The currently available build flags are listed below:

- `LOW_RES_TIMER`: disables all high-resolution timers and replaces them with a
                   low-resolution (seconds only), but portable timer. For Linux
                   as Windows has guaranteed high-resolution timer support, but
                   this flag should still take effect under Windows.

Troubleshooting
---------------

- **Q**: I am on Windows and the compiler is complaining about `(n)curses.h`.

  **A**: `ncurses` is Linux-only. But fear not, a Windows implementation exists
         under the name `PDCurses`. You should be able to find it online - note
         I did not find a prebuilt 64-bit version of this library, so you might
         have to build it yourself if you want 64-bit support.
         Now you have the library installed in your favorite compiler, you just
         need to replace `ncurses.h` by `curses.h`, and link to `PDCurses`, not
         `ncurses`, and everthing should now work.

- **Q**: I am getting many compiler errors, regarding various OpenCL functions,
         such as undefined references, undefined types, and so on.

  **A**: Make sure you are defining `CL_USE_DEPRECATED_OPENCL_1_1_APIS` in your
         compiler options. Also make sure you are linking to OpenCL.

- **Q**: The renderer builds successfully, however, I get a CLC build error, as
         follows: `#include <cl/epsilon.cl> file not found`.

  **A**: This means the OpenCL compiler can't find the kernel files - this is a
         known issue for the APP OpenCL runtime, and the fix is as follows:

1. Go into `src/engine/renderer.cpp`, find the offending line and rename
   the line with the absolute path (on your system) to `epsilon.cl`. Use
   quotes instead of angled brackets, e.g `#include "/path/to/epsilon.cl"`

2. Do the same for every `#include` directive found in the `cl/` folder.

3. Rebuild the renderer, and try again, the error should not occur.

Why does this happen? We don't know - APP seems to have issues regarding
include directives, and we cannot reasonably fix this in the renderer.

Another reported fix, is to add the `cl/` directory to your system PATH,
which should technically fix the error in a less (or more, depending on
your perspective) intrusive manner.

- **Q**: I tried the renderer on my processor, it worked. But when I try to run
         it on the GPU, it freezes and nothing happens (or my screen crashes)!

  **A**: The current kernel is implemented as an infinite loop for convenience,
         which can cause problems with GPU's. To remedy this, simply change the
         loop  into a `for` construct with, say, 8 or 9 iterations. Try again.

- **Q**: I'm having incomprehensible compilation errors under Windows.

  **A**: I've encountered a situation, under Windows, where many variable names
         used by the renderer happen to be macros under MinGW, for instance the
         names `far`, `near`, `interface`, etc... If everything else fails, try
         renaming those variables something else. But this really should not be
         happening and a compiler misconfiguration could be the cause.

License
-------

The code is released under the MIT license. You are free to use my code in your
own projects, both open-source and commercial, but if you use substantial parts
of it, please retain the provided license file somewhere in your project.

Substantial means the entire OpenCL kernel, or half the source files, that sort
of stuff. Just be reasonable, most of the renderer's value lies in the concepts
involved anyway, and not in the code itself.
