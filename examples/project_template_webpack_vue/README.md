# Using this template project

## Build

First, copy this folder `project_template_webpack_vue` to other place.

Secondly, run this command(build Luna bin).

```
conan install .
conan build .
```

Move `assets` folder, and install library(for yarn)

```
cd assets
yarn install
yarn build
```

Run server 

```
cd .. && ./bin/awesomesauce
```

Access to  `localhost:8080`, you can see SPA sample!