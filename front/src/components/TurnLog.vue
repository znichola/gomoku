<script setup lang="ts">
import { computed } from 'vue';
import { useGameStore } from '@/stores/game'

const gameStore = useGameStore()

const moves = computed(() => gameStore.gameState.moveHistory)
</script>

<template>
  <div class="turnlog">
    <h2>Turn log</h2>
    <ul>
      <li v-for="(move, i) in moves"
        :key="move"
         @mouseover="gameStore.highlight.set(move, true)" @mouseleave="gameStore.highlight.set(move, false)"
        :class="i % 2 === 0 ? 'white' : 'black'"
        >{{ move }}</li>
    </ul>
  </div>
</template>

<style scoped land="less">
h2 {
  padding-bottom: 0.4rem;
}
.turnlog {
  color: var(--line-color);
  font-family: 'Courier New', Courier, monospace;
  margin: 0 calc(var(--cell-size) / 2);
  max-height: 100%;
  overflow-y: auto;
  ul {
    display: flex;
    flex-wrap: wrap;
  }
  ul li {
    padding: 0.4rem 1rem;
    color: var(--primary-color);
    border: 1px solid var(--line-color);
    cursor: pointer;
    &:nth-child(even) {
      background: var(--primary-color);
      color: var(--bg-color);
    }
    &:hover, &.reverse {
      background: var(--accent-color);
    }
  }
}

.white {
  color: var(--white-color);
}

.black {
  color: var(--black-color);
}
</style>
