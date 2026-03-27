<script setup lang="ts">
import { computed } from 'vue';
import { useGameStore } from '@/stores/game'

const gameStore: gameStore = useGameStore()

const moves = computed(() => gameStore.gameState.moveHistory)
const boardDimentions = computed(() => gameStore.gameState.board?.boardDimentions || 19)
</script>

<template>
  <div class="turn-log">
    <h2>Turn log</h2>
    <div class="move-list">
      <span v-for="(move, i) in moves"
        :key="move"
        class="turn"
        :class="i % 2 === 0 ? 'white' : 'black'"
        >{{ move }}</span>
    </div>
  </div>
</template>

<style scoped land="less">
.turn-log {
  color: var(--line-color);
  font-family: 'Courier New', Courier, monospace;
  margin-left: 3rem;
}

.move-list {
  display: flex;
  flex-wrap: wrap;
}

.white {
  color: var(--white-color);
}

.black {
  color: var(--black-color);
}

.turn {
  border: solid 1px var(--line-color);  
  padding: 0.2rem;
  min-width: 2ch;
}

</style>