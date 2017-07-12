---
layout: default
title: Using Luna with your project
---

# Using Luna with your project

Luna has been built and tested on macOS and Linux. (I'd like some help getting it working on Windows, if you have the time and inclination!)

Luna also depends upon the CMake build system, and the Conan dependency manager.

## Installing dependencies with Conan

Luna is available from (and makes use of the) the [Conan](https://www.conan.io) dependency manager. And so should you. It's pretty good.

To incorporate Luna into your project using Conan, first execute the following command to add the luna repository to a place that Conan can find it:
                                                         
```
$ conan remote add DEGoodmanWilson https://api.bintray.com/conan/degoodmanwilson/opensource
```

Then create a file called `conanfile.txt`, and add the following:


```
[requires]
luna/{{ site.version }}@DEGoodmanWilson/stable
```

If you are using [more advanced Conan features](http://docs.conan.io/en/latest/conanfile_py.html), you can just add this to your `conanfile.py`

```python
class MyAwesomeProject(ConanFile)
    requires = "luna/{{ site.version }}@DEGoodmanWilson/stable"
    ...
```

At any rate, _from your build directory_, simply run

```bash
$ conan install --build=missing path/to/source
```

(The `--build=missing` is because at the moment Luna only offers source installs&emdash;we'll offer pre-built binaries before too long, just you see.)

(You can read more about using Conan with your project [in the Conan docs](http://docs.conan.io/en/latest/))

### Conan options

You can fiddle with the following options for your build:
 
* `build_shared_libs` Build Luna as a dynamic/shared library. Default is to build as a static library. Building shared libraries is basically untested at the moment.

The following options exist, but only really affect development of Luna

* `build_luna_tests` Build the Luna test suite. Default is not to.
* `build_luna_examples` Build the included Luna example projects. Default is not to.
* `build_luna_coverage` Build the Luna coverage suite. Implies `build_luna_tests=True`. Default is not to.

## CMake

If you've never used Conan before, it works with a wide range of project toolchains, but Conan works best with CMake. You can specify the `cmake` generator in the `conanfile.txt`:

```
[generators]
cmake
```

Then add the following lines somewhere near the top of your `CMakeLists.txt`:

```cmake
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()
```

Then you can run CMake as usual

```bash
cmake path/to/source
cmake --build .
```
