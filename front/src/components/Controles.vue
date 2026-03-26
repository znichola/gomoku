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

</script>

<template>
<button class="reset-btn" @click="reset">Restart</button>
</template>

<style scoped lang="less">

button {
  padding: 0.6rem;
  outline: solid 3px var(--accent-color);
  border-radius: 1.2rem;
  background-color: var(--primary-color);
  color: var(--bg-color);
  font-weight: bolder;
  font-size: large;
}

</style>