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

You probably know better how to do this than I do. With your chosen package manager, make sure you have the following installed:

* `gcc` >= 4.9 (6 is ideal) _or_ `clang` >= 3.6 (your pick. If you aren't sure, choose gcc)
* `cmake` >= 2.8

To install Conan, [follow the directions on the Conan site](https://www.conan.io/downloads).

Try it out:

```shell
conan --version
```

# Building your first project

OK, great you have the tools installed! Let's build the examples that come with Luna.

I assume you've got a copy of this repo somewhere handy. Open a terminal window, and `cd` into the project root directory.

## Install dependencies

First, let's install all of Luna's dependencies. The first time you do this, it will take a while. Maybe even a long while. But future runs will go much much faster.

The first step is to add the remote host containing Luna to Conan. You'll only need to do this once, ever.
 
```shell
conan remote add vthiery https://api.bintray.com/conan/vthiery/conan-packages
conan remote add degoodmanwilson https://api.bintray.com/conan/degoodmanwilson/opensource
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
```

Then install and build the dependencies like this. The additional option at the end also creates the optional Luna examples.

```shell
conan install . -o build_luna_examples=True
```

Rest assured that there are pre-built Docker images that you can use in the future to avoid this long step when it comes time to deploy. We'll come to that in the next section.

## Build the project

Once conan has done its thing, we can use conan to also build the project. From the the project directory run:

```shell
conan build .
```

## Trying it out

When conan is done building your project, you'll find all kinds of goodies in the folder `bin`. Try running `bin/basic_webapp`:

```shell
./bin/basic_webapp
```

Then visit [localhost:8443/parameters?foo=bar&baz=qux](localhost:8443/parameters?foo=bar&baz=qux), and you should see some basic HTML in your browser.

In the next section, we'll look at how the code that drives this page works, so you can start to add your own functionality.

----

### < [Prev—Home](index.html) | [Next—Simple API endpoints](simple_api_endpoint.html) >
