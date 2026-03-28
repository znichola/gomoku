<script setup lang="ts">
import { useGameStore } from '@/stores/game'

const gameStore = useGameStore()

import controles from './Controles.vue'
import game from "./Game.vue"
import hud from './Hud.vue'
import turnlog from './TurnLog.vue'

</script>

<template>
<div class="layout">
  <div class="header">
    <div id="logo">
      <h1>Gomoku</h1>
      <span class="desc" v-if="gameStore.watcherState.enabled">[DEBUG] Watcher is enabled</span>
    </div>
    <controles />
    <hud />
  </div>
  <game class="game" />
  <turnlog class="turnlog" />
</div>
</template>

<style scoped lang="less">
h1 {
  cursor: pointer;
}

.layout {
  max-width: var(--content-width);
}

.header {
  display: flex;
  align-items: center;
  justify-content: center;
}

#logo {
  position: relative;
  .desc {
    font-size: 0.8rem;
    position: absolute;
    right: -1rem;
    bottom: -0.2rem;
    color: white;
  }
}

.turnlog {
}

</style>
