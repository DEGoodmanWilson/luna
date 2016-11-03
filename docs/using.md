---
layout: default
title: Using Luna with your project
---

# Using Luna with your project

Luna has been built and tested on macOS and Linux. (I'd like some help getting it working on Windows, if you have the time and inclination!)

Luna uses libmicrohttpd, so please make sure you have that installed first:

```bash
# Debian, Ubuntu, &c.
$ sudo apt-get install libmicrohttpd-dev

# Mac
$ brew install libmicrohttpd 
```

Luna also depends upon the cmake build system, and optionally (and ideally) Conan.

## Conan (preferred)

Luna is available from (and makes use of the) the [Conan](https://www.conan.io) dependency manager. And so should you.

To incorprate Luna into your project using Conan, simply add the following to your `conanfile.txt`:

```
[requires]
luna/1.0.0@DEGoodmanWilson/stable
```
    
or if you are using the Pythonic version, add this to your `conanfile.py`

```python
class MyAwesomeProject(ConanFile)
    requires = "luna/1.0.0@DEGoodmanWilson/stable"
    ...
```

Then run

```bash
$ conan install --build=missing
```

(The `--build=missing` is because at the moment luna only offers source installs&emdash;we'll offer pre-built binaries before too long, just you see.)

(You can read more about using Conan with your project [in the Conan docs](http://docs.conan.io/en/latest/))

### Conan options

You can fiddle with the following options for your build:
 
* `build_shared_libs` Build Luna as a dynamic/shared library. Default is to build as a static library. This is basically untested at the moment.

The following options exist, but only really affect development of Luna

* `build_luna_tests` Build the Luna test suite. Default is not to.
* `build_luna_examples` Build the included Luna example projects. Default is not to.
* `build_luna_coverage` Build the Luna coverage suite. Implies `build_luna_tests=True`. Default is not to.


## Raw source

Of course, you can just compile Luna into a library, and link that with your project. That's fine too!

Luna uses cmake as the build system, so building is reasonably straightforward. Start by making a directory for building Luna.

```bash
$ mkdir build
$ cmake path/to/luna -DCMAKE_BUILD_TYPE=Release #or Debug if you prefer
$ cmake --build .
```

Then just be sure to include the path to the Luna project in your include search path, the path to the resulting library file in your linker search path!