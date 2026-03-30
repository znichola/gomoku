<script setup lang="ts">
import type { RefStringOrNull } from '@/types/vue'
import { computed, onMounted, onUnmounted, ref } from 'vue'
import type { Cell } from '@/types/game'
import { getCellClass } from '@/helpers/helpers'
import { useGameStore } from '@/stores/game'

const errorMessage: RefStringOrNull = ref(null)
const gameStore = useGameStore()

const boardDimension = computed(() => gameStore.gameState.board?.boardDimension || 19)
const grids = computed(() => {
  const arr: number[][] = []
  for (let y = 0; y < boardDimension.value; y++) {
    arr[y] = []
    for (let x = 0; x < boardDimension.value; x++) {
      arr[y]![x] = x + y * boardDimension.value
    }
  }
  return arr
})

const previewGrid = computed(() => gameStore.watcherState.preview_state)

onMounted(load)
onUnmounted(() => {
  clearTimeout(_load_timeout)
  gameStore.backWatcher('unMounted')
})

let _load_timeout = 0
async function load(error: boolean = false) {
  errorMessage.value = ''
  try {
    const resp = await fetch(`http://${window.location.hostname}:9012/gameState?silent&firstload`)
    if (resp.status != 200)
      throw Error('STATUS NOT 200')
    const watcher = gameStore.backWatcher('mounted')
    const data = await resp.json()
    if (watcher.checkResponse(data, resp))
      gameStore.updateGameState(data)
    if (gameStore.watcherState.enabled && error)
      watcher.applyT0()
  } catch (err) {
    clearTimeout(_load_timeout)
    _load_timeout = setTimeout(load, 1000, true)
    // errorMessage.value = 'NO error rescued, but something went wrong !'
    console.warn((err as Error).message)
  }
}

async function move(event: MouseEvent) {
  const target = event.target as HTMLElement
  const cellId = target.id
  errorMessage.value = ''
  console.log(`Clicked on cell ${cellId}`)
  try {
    const resp = await fetch(`http://${window.location.hostname}:9012/move?id=${cellId}`)
    if (resp.status != 200)
      throw Error('STATUS NOT 200')
    const data = await resp.json()
    if (gameStore.backWatcher().checkResponse(data, resp))
      gameStore.updateGameState(data)
  } catch (err) {
    errorMessage.value = 'NO error rescued, but something went wrong !'
    console.warn((err as Error).message)
  }
}

</script>

<template>
  <section>
    <p class="error" v-if="errorMessage">Message : {{ errorMessage }}</p>
    <!-- Gameboard -->
    <div class="board"
      :style="{
        '--turn-color': gameStore.gameState.board?.isBlackToPlay
          ? 'var(--black-color)'
          : 'var(--white-color)'}"
      >
      <template v-if="(gameStore.watcherState.preview || (gameStore.watcherState.enabled && !gameStore.gameState.board)) && previewGrid.length > 0">
        <div v-for="(line, y) in grids" :key="y" class="line preview">
          <div v-for="(cid, x) in line" :key="x" class="cell">
            <div class="circle"
            :key="cid.toString()"
            :data-type="getCellClass(previewGrid[cid] as Cell)"
            :title="`[${x}; ${y}] - id: ${cid}`"
            :id="cid.toString()"
              ></div>
          </div>
        </div>
      </template>
      <template v-else-if="gameStore.gameState.board">
        <div v-for="(line, y) in grids" :key="y" class="line">
          <div v-for="(cid, x) in line" :key="x" class="cell">
            <div class="circle"
            :class="{ highlight: gameStore.highlight.get() == cid }"
            :data-type="getCellClass(gameStore.gameState.board?.grid[cid] as Cell)"
            :title="`[${x}; ${y}] - id: ${cid}`"
            :id="cid.toString()"
            @click="move"
              ></div>
          </div>
        </div>
      </template>
    </div>
  </section>
</template>

<style scoped lang="less">
p.error {
    background: indianred;
    padding: 1.2rem;
    text-align: center;
    color: white;
}

div.board {
  // --celsize: min(40px, calc(100vw / 19));
  margin-top: calc(var(--celsize) / -2);
  margin-left: calc(var(--celsize) / -2);
  padding-bottom: calc(var(--celsize) / 2);
  padding-right: calc(var(--celsize) / 2);
  box-sizing: content-box;

  div.line {
    display: flex;

    &.preview div.cell {
      border-right: 1px solid green;
      border-bottom: 1px solid green;
    }
  }

  div.cell {
    width: var(--celsize);
    height: var(--celsize);
    line-height: var(--celsize / 2);
    border-right: 1px solid var(--line-color);
    border-bottom: 1px solid var(--line-color);
    text-align: center;

    position: relative;
    div.circle {
      --radius: calc(var(--celsize) / 1.5);
      position: absolute;
      left: calc(var(--celsize) - var(--radius) / 2);
      top: calc(var(--celsize) - var(--radius) / 2);
      width: calc(var(--radius));
      height: var(--radius);
      line-height: var(--radius);
      font-size: 8px;
      border-radius: var(--celsize);
      padding-top: 2px;
      background-color: var(--line-color);
      color: var(--text-color);
      text-align: center;
      cursor: pointer;
      z-index: 200;
      &:hover {
        background-color: var(--turn-color);
      }

      &.highlight {
        animation: scale-highlight 1s ease-in-out infinite;
        &[data-type=empty] {
          opacity: 1;
          transition: all 0.2s ease-in;
        }
      }

      @keyframes scale-highlight {
        from {
          transform: scale(1.2);
        }
        50% {
          transform: scale(1.4);
        }
        to {
          transform: scale(1.2);
        }
      }

      &[data-type=black], &[data-type=white] {
        --radius: calc(var(--celsize) * 0.75);

        background: var(--white-color);
        &::after {
          position: absolute;
          left: 0;
          top: 0;
          width: calc(var(--radius) * 0.8);
          height: calc(var(--radius) * 0.8);
          margin: calc(var(--radius) * 0.1);
          border: calc(var(--radius) * 0.08) solid var(--bg-color);
          border-radius: var(--celsize);
          box-sizing: border-box;
          content: ' ';
          z-index: 201;
        }
      }
      &[data-type=black] {
        background: var(--black-color);
        color: var(--white-color);
      }
      &[data-type=empty] {
        opacity: 0;
        transition: all 0.2s ease-in;
      }
      &[data-type=empty]:hover {
        opacity: 1;
      }
    }
  }

  div.preview div.circle[data-type=empty] {
    display: none;
  }

  div.line > div:nth-child(1).cell:nth-child(1) {
    border-bottom: none;
  }

  div.line:nth-child(1) > div.cell {
    border-right: none;
  }
}
</style>
