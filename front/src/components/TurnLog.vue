<script setup lang="ts">
import { computed, ref, watch } from 'vue';
import { useGameStore } from '@/stores/game'

const gameStore = useGameStore()

const moves = computed(() => gameStore.gameState.moveHistory)

const savedMoves = ref<string[] | null>(null)

// Clear redo once a new move is played beyond the replayed state
watch(moves, (newMoves, oldMoves) => {
  if (savedMoves.value && newMoves.length > oldMoves.length) {
    savedMoves.value = null
  }
})

async function replayMoves(turnIndex: number) {
  try {
    // Save the full history before rewinding (only on first rewind)
    if (!savedMoves.value) {
      savedMoves.value = [...gameStore.gameState.moveHistory]
    }
    const moveSlice = gameStore.gameState.moveHistory.slice(0, turnIndex).join(',');
    const resp = await fetch(`http://${window.location.hostname}:9012/replay?moves=${moveSlice}`, {
      method: 'GET',
    })
    const data = await resp.json()
    gameStore.updateGameState(data);
  } catch (err) {
    console.warn(err)
  }
}

async function redoMoves() {
  if (!savedMoves.value) return
  try {
    const resp = await fetch(`http://${window.location.hostname}:9012/replay?moves=${savedMoves.value.join(',')}`, {
      method: 'GET',
    })
    const data = await resp.json()
    gameStore.updateGameState(data)
    savedMoves.value = null
  } catch (err) {
    console.warn(err)
  }
}
</script>

<template>
  <div class="turnlog">
    <h2>Turn log</h2>
    <ul>
      <li v-for="(move, i) in moves"
        :key="move"
         @mouseover="gameStore.highlight.set(move, true)" @mouseleave="gameStore.highlight.set(move, false)"
         @click="replayMoves(i)"
        :class="i % 2 === 0 ? 'white' : 'black'"
        >{{ move }}</li>
        <li v-if="savedMoves" @click="redoMoves">Redo ↷</li>
    </ul>
  </div>
</template>

<style scoped lang="less">
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
