<script setup lang="ts">
import { computed, ref, watch } from 'vue';
import { useGameStore } from '@/stores/game'

const gameStore = useGameStore()

const moves = computed(() => gameStore.gameState.moveHistory)

const savedMoves = ref<number[] | null>(null)

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
    <h2>Turn log [{{ Math.floor((moves.length) / 2) }}]</h2>
    <div class="scroll">
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
    <div class="bottom-spacer"></div>
  </div>
</template>

<style scoped lang="less">
h2 {
  padding-bottom: 0.4rem;
  font-size: 1rem;
  text-align: center;
  color: var(--primary-color);
}
.turnlog {
  display: flex;
  flex-direction: column;
  color: var(--line-color);
  font-family: 'Courier New', Courier, monospace;
  box-sizing: border-box;
  width: 100%;
  max-width: 16rem;
  margin: 0.5rem auto 0;
  height: 100%;
  padding: 0.25rem;
  overflow: auto;

  h2 {
    flex: none;
  }

  .scroll {
    flex: 1;
    min-height: 0;
    overflow-y: auto;
  }

  .bottom-spacer {
    padding: 0.7rem 0;
  }

  ul {
    display: flex;
    flex-wrap: wrap;
  }
  ul li {
    padding: 0.25rem 0.6rem;
    color: var(--primary-color);
    border: 1px solid var(--line-color);
    text-align: center;
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

// Narrow screens: turnlog is stacked below the board instead of docked in a
// sidebar, so let it use the full available width, but cap its own height
// (instead of following the page) so it scrolls internally, not endlessly.
@media all and (max-width: 899.98px) and (min-height: 600.02px) {
  .turnlog {
    max-width: none;
    height: auto;
    max-height: 10rem;
  }
}
</style>
