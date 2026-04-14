<script setup lang="ts">
import { useGameStore } from '@/stores/game'

const gameStore = useGameStore()

import controles from './Controles.vue'
import game from "./Game.vue"
import hud from './Hud.vue'
import turnlog from './TurnLog.vue'
import messages from './Messages.vue'

const reload = () => window.location.reload()
</script>

<template>
  <div class="layout">
  <div class="header">
    <div id="logo">
      <h1 @click="reload">Gomoku</h1>
      <span class="desc" v-if="gameStore.watcherState.enabled">[DEBUG] Watcher is enabled</span>
    </div>
    <controles />
    <hud />
    <messages />
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
  flex-wrap: wrap;
}

#logo {
  position: relative;
  .desc {
    font-size: 0.8rem;
    position: absolute;
    right: -1rem;
    bottom: -0.2rem;
    color: var(--white-color);
  }
}

h1::after {
  content: '✏️';
  position: absolute;
  font-size: 2rem;
  right: -1.5rem;
  top: -3rem;
  transition: all 0.5s ease-in-out;
}

.editMode h1::after {
  top: 0;
}

.turnlog {
}

</style>
