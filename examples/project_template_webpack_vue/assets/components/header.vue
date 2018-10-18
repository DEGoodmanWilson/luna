<template>
<div>
<nav class="navbar navbar-expand-lg navbar-dark bg-primary">
  <a class="navbar-brand" href="#">LunaWithVue</a>
  <div class="dropdown">
    <button class="btn btn-secondary dropdown-toggle" type="button" id="dropdownMenuButton" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
      Menu
    </button>
    <div class="dropdown-menu" aria-labelledby="dropdownMenuButton">
      <router-link to="/" class="dropdown-item">Top</router-link>
      <router-link to="/about" class="dropdown-item">About</router-link>
      <router-link to="/contact" class="dropdown-item">Contact</router-link>
    </div>
  </div>
</nav>
</div>    
</template>