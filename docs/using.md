---
layout: default
title: Getting started with Luna
---

# Getting started with Luna

Let's build the example app `basic_webapp`, found in `examples/basic_webapp.cpp`. This is a simple example app that demonstrates many of Luna's features. We'll learn how to build the app, and add a new endpoint to it. All the code in that app is production ready, so feel free to borrow any of it for your own projects. We'll cover scaffolding your own project in the [next tutorial](scaffolding.html).

Luna requires a few tools to build; if you're new to C++ you might not have some or all of them on your syste. First, of course, you need a C++ compiler. You'll also need the CMake build system, and the Conan dependency manager.

## Installing build tools on Mac

Clang (a C++ compiler) and CMake are both bundled with Xcode. You don't need to install all of Xcode, we only need the command line tools. In the future, if you're looking for a powerful IDE, you can choose to install Xcode, although I personally recommend JetBrains CLion.

Just run this command in your terminal to install the tools

```shell
xcode-select --install
```

You can test it out by running this command:

```shell
clang --version
```

Conan is a relative newcomer to the C++ world. It works a lot like dependency managers you may have used before, like NPM, PyPI, or RubyGems. The best way to install Conan is with [homebrew](https://brew.sh).

```shell
brew install conan
```

Easy peasy. Try it out:

```shell
conan --version
```

Conan is update very frequently, and I strongly recommend you keep it up to date.
```shell
brew upgrade conan
```

## Installing build tools on Linux

# Building your first project

## Install dependencies

## Build the project

## Trying it out

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

If you've never used Conan before, it works with a wide range of project toolchains, but Conan works best with CMake. You can specify the `cmake` generator in the `conanfile.txt` before running `conan install`:

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
