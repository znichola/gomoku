<script setup lang="ts">
import { useGameStore } from '@/stores/game'

const gameStore = useGameStore()

async function reset() {
  try {
    const resp = await fetch(`http://localhost:9012/reset`, {
      method: 'GET',
    })
    const data = await resp.json()
    console.log(data)
    gameStore.updateGameState(data);
    } catch (err) {
    console.warn(err)
  }
}

async function debug(action: string) {
  try {
    const resp = await fetch(`http://localhost:9012/debug-action?action=${action}`, {
      method: 'GET',
    })
    const data = await resp.json()
    console.log(data)
    gameStore.updateGameState(data);
    } catch (err) {
    console.warn(err)
  }
}

function watcher(action: string) {
  if (action === 'start') {
    gameStore.backWatcher('mounted')
  } else if (action === 'set-t0') {
    gameStore.backWatcher().setT0()
  }
}

</script>

<template>
<div class="controles">
  <button class="reset-btn" @click="reset">Restart</button>
  <div class="menu">
    <span>Débug</span>
    <ul>
      <li><button class="debug-btn" @click="debug('make-double-tree')">Double Tree Maker</button></li>
      <li><button class="debug-btn" @click="debug('set-reset')">Set Restart</button></li>
      <li><button class="debug-btn" @click="watcher('start')">Start watcher</button></li>
      <li><button class="debug-btn" @click="watcher('set-t0')">Watcher Set T0</button></li>
    </ul>
  </div>
</div>
</template>

<style scoped lang="less">

.controles {
  padding: 0.3em;
  display: flex;
  align-items: center;
  gap: 1rem;
  margin-left: 2rem;
}

div.menu {
  position: relative;
  ul {
    display: none;
    position: absolute;
    padding-top: 1rem;
    li {
      width: max-content;
      padding-bottom: 1rem;
    }
  }
  &:hover ul, & ul:hover {
    display: block;
    z-index: 400;
  }
}

button, div.menu span {
  cursor: pointer;
  display: block;
  padding: 0.2rem 0.3rem;;
  outline: solid 0.165rem var(--accent-color);
  border-radius: 1.2rem;
  background-color: var(--primary-color);
  color: var(--bg-color);
  font: var(--ui-font);
  transition: all 0.3s ease;

  &:hover {
    background-color: var(--accent-color);
    color: var(--primary-color);
  }

  &:active {
    transform: scale(0.95);
  }
}

</style>
