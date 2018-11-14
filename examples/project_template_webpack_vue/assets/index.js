import Vue from 'vue/dist/vue.esm';
import Router from './router/router'
import Header from './components/header.vue';
import * as  Bootstrap from 'bootstrap';
import 'bootstrap/dist/css/bootstrap.css';

Vue.use(Bootstrap)

const hello = new Vue({
    router: Router,
    el: '#app',
    components: {
        'nav-bar': Header
    }
})