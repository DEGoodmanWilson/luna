# Using the project template

Included with the Luna source code is a fully functional project template you can use to start your own Luna-built API. Look in `examples/project_template` for the code.

Let's build a project called `awesomesauce` (because I'm feeling super creative at the moment). `awesomesauce` is the default name in the project template—if you want to change your project name, you'll need to do a global search and replace on `awesomesauce`.

## Building locally

First, copy the folder `examples/project_template` to its own folder, `foobar`, located somewhere handy.

From a terminal window, let's install the dependencies.

```shell
conan install --build=missing
```

This might take a while, that's OK. You only need to build the dependencies once.

Building the sample code is easy. First, let's tell CMake to build the project files

```shell
cmake .
```

Then, we can build the project like this:

```shell
cmake --build .
```

Run the example with 
```shell
./bin/awesomesauce
```

## Docker support

There's a Dockerfile you can try out too, that will build and run the `awesomesauce` binary. (The port used by the demonstration code is 8080.)

```shell
docker build -t awesomesauce .
docker run -p 8080:8080 awesomesauce
```

Now visit [http://localhost:8080](http://localhost:8080) to see the app in action!
