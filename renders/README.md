Gallery
==============

These are some interesting or noteworthy renders produced by εpsilon.

<p align="center">
<img src="https://raw.github.com/TomCrypto/epsilon/master/renders/staircase.png" alt="First complex render"/>
</p>
This was the first scene with more than a dozen triangles rendered when the BVH
was ported to the OpenCL kernel. Here mostly for historical reasons.

<p align="center">
<img src="https://raw.github.com/TomCrypto/epsilon/master/renders/dragon.png" alt="Diffuse Dragon/Lucy"/>
</p>
Diffuse dragon + Lucy scene, with 625k triangles. This was to test the BVH, and
was a (relative) success. The colors were scaled back a bit for aesthetics.

<p align="center">
<img src="https://raw.github.com/TomCrypto/epsilon/master/renders/depth_of_field.png" alt="Depth-of-Field"/>
</p>
This is a render, showing the depth-of-field effect, with the staircase's cross
section in focus. The effect was exaggerated (by purpose or by accident... your
choice!)

<p align="center">
<img src="https://raw.github.com/TomCrypto/epsilon/master/renders/depth_of_field_open.png" alt="Outdoor Depth-of-Field"/>
</p>
This is an outdoor scene showing the depth-of-field effect even more clearly...
Sorry for abusing the staircase model, but it is a rather good test scene.

<p align="center">
<img src="https://raw.github.com/TomCrypto/epsilon/master/renders/absorption.png" alt="Spectral Absorption"/>
</p>
This is a new model (380k triangles), which I used to demonstrate the brand new
material system, and in particular spectral absorption. The glass interface has
no influence on the Buddha's red color, which is entirely due to the absorption
of non-red wavelengths by the statue's medium. Strangely reminiscent of a gummy
bear.

<p align="center">
<img src="https://raw.github.com/TomCrypto/epsilon/master/renders/buddha.png" alt="Light Buddha"/>
</p>
This is a curious render. Basically, it is the previous render's buddha statue,
with more or less the same absorbing material, and inside it, the same model is
duplicated with a slightly smaller scale, and as a light source instead. As you
can see, the results are interesting. Note the caustic on the ceiling.
