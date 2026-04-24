<script setup lang="ts">
import { ref } from 'vue'
import { useGameStore } from '@/stores/game'

const gameStore = useGameStore()

import controles from './Controles.vue'
import game from "./Game.vue"
import hud from './Hud.vue'
import turnlog from './TurnLog.vue'
import messages from './Messages.vue'
import RulesModal from './RulesModal.vue'
import WinnerModal from './WinnerModal.vue'

const reload = () => window.location.reload()

const rulesOpen = ref(false)

async function reset() {
  try {
    const resp = await fetch(`http://${window.location.hostname}:9012/reset`, {
      method: 'GET',
    })
    const data = await resp.json()
    gameStore.updateGameState(data);
    } catch (err) {
    console.warn(err)
  }
}
</script>

<template>
  <div class="layout">
  <div class="header">
    <div id="logo">
      <h1 @click="reload">Gomoku</h1>
      <span class="desc" v-if="gameStore.watcherState.enabled">[DEBUG] Watcher is enabled</span>
      <!-- Bouton règles -->
      <span class="rules" type="button" @click="rulesOpen = true">?</span>
    </div>

    <controles />
    <hud />
    <messages />
  </div>
  <game class="game" />
  <turnlog class="turnlog" />
</div>
<RulesModal :open="rulesOpen" @close="rulesOpen = false" />
<WinnerModal @close="reset()" />
</template>

<style scoped lang="less">
h1 {
  cursor: pointer;
}

.layout {
  max-width: var(--content-width);
  height: 100vh;
}

.header {
  height: min-content;
  display: flex;
  align-items: center;
  justify-content: center;
  flex-wrap: wrap;
  padding-top: 0.8rem;
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

.header .rules {
  cursor: pointer;
  position: absolute;
  top: 0px;
  left: -20px;
  padding: 5px;
  font-size: 1.4rem;
  font-weight: bold;
  color: var(--text-color);
}

h1::after {
  content: '✏️';
  position: absolute;
  font-size: 2rem;
  right: -1.5rem;
  top: -3rem;
  transition: all 0.5s ease-in-out;
}

h1::before {
  content: '⌨️';
  position: absolute;
  font-size: 1.8rem;
  left: 0rem;
  bottom: 6rem;
  transition: all 0.5s ease-in-out;
}

.editMode h1::after {
  top: 0;
}

.keyMode h1::before {
  bottom: -1rem;
}
</style>
