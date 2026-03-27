<script setup lang="ts">
import controles from './Controles.vue'
import game from "./Game.vue"
import hud from './Hud.vue'

</script>

<template>
  <div class="header">
    <h1>Gomoku</h1>
    <controles/>
    <hud/>
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