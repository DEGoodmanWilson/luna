<template>
<div>
    <h1>Index Pages</h1>
    <p v-for="(item, key, index) in items" :key=index>
        {{item}}
    </p>

    <div class="input-group">
        <div class="input-group-append">
            <span class="input-group-text">text</span>
        </div>
        <input type="text" class="form-control" v-model="text" placeholder="input text!"> 
    </div>
    <p>
        <button type="button" class="btn btn-primary" v-on:click="postApi">Submit</button>
    </p>
</div>    
</template>

<script>

import axios from 'axios';

export default {
    data: function() {
        return {
            items: [],
            text: ""
        }
    },
    mounted: function() {
        this.getApi();
    },
    methods: {
        getApi: function() {
            axios.get('api/endpoint').then((response) => {
                const data = Object.values(response.data);
                for(var i = 0; i < data.length; i++){
                    this.items.push(data[i]);
                }
            }, (error) => {
                console.log(error);
            });
            console.log(this.items);
        },
        postApi: function() {
            var params = new URLSearchParams();
            params.append('text', this.text);
            axios.post('/api/post', params).then((response) => {

                this.items.push(this.text);
                this.text = "";
                console.log(response);
                this.$forceUpdate();

            }, (error) => {
                console.log(error);
            });
        },
    }
}

</script>
