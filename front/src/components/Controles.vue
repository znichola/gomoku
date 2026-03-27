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
<div class="controles">
  <button class="reset-btn" @click="reset">Restart</button>
</div>
</template>

<style scoped lang="less">

.controles {
  padding: 0.3em;
  display: flex;
  align-items: center;
  margin-left: 2rem;
}

button {
  padding: 0.2rem 0.3rem;;
  outline: solid 0.165rem var(--accent-color);
  border-radius: 1.2rem;
  background-color: var(--primary-color);
  color: var(--bg-color);
  font: var(--ui-font);
}

</style>