<script setup lang="ts">
import controles from './Controles.vue'
import game from "./Game.vue"
import hud from './Hud.vue'
import turnlog from './TurnLog.vue'

</script>

<template>
<div class="layout">
  <div class="header">
    <h1>Gomoku</h1>
    <controles />
    <hud />
  </div>
  <game class="game" />
  <turnlog class="turnlog" />
</div>
</template>

<style scoped lang="less">
.layout {
  max-width: var(--content-width);
}

.header {
  display: flex;
  align-items: center;
  justify-content: center;
  grid-area: header;
  h1 {
    padding-bottom: 1.2rem;
  }
}

.game {
  margin-top: -2.5rem;
}

.turnlog {
}

</style>