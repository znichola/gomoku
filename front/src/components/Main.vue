<script setup lang="ts">
import controles from './Controles.vue'
import game from "./Game.vue";

</script>

<template>
  <div class="header">
    <h1>Gomoku</h1>
      <controles/>
  </div>
  <game/>

</template>

<style scoped lang="less">
.header {
  display: flex;
  align-content: center;
  justify-content: center;
}
</style>