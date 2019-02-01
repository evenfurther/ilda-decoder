ILDA decoder
============

This library implements a pull-mode ILDA decoder which does not do any dynamic allocation
and is suitable for embedded systems.

Dependencies
------------

This library does not depend on anything by itself (just add the `ilda-decoder.h` and
`ilda-decoder.c` files in your project).

However, if you want to build the utility programs, you will need a recent
[meson](https://mesonbuild.com), and the [SDL2](https://www.libsdl.org/)
library if you want to build the `ilda-display` renderer.

License
-------

This library is released under a dual MIT/Apache 2.0 license.

Building and installing on a non-embedded system
-------------------------------------------------

To build and test the library and executables, use:

``` bash
$ mkdir _build
$ cd _build
$ meson setup --opt 3 ..
$ ninja
```

To install it, add:

``` bash
$ sudo ninja install
```

Please refer to the Meson documentation if you want to configure the build process
or the installation paths.
