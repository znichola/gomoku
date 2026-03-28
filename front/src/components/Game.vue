<script setup lang="ts">
import type { RefStringOrNull } from '@/types/vue'
import { computed, onMounted, onUnmounted, ref } from 'vue'
import type { Cell } from '@/types/game'
import { getCellClass } from '@/helpers/helpers'
import { useGameStore } from '@/stores/game'

const errorMessage: RefStringOrNull = ref(null)
const gameStore = useGameStore()

const boardDimension = computed(() => gameStore.gameState.board?.boardDimension || 19)

onMounted(load)
onUnmounted(() => gameStore.backWatcher('unMounted'))

async function load() {
  // const watcher = gameStore.backWatcher('mounted')
  errorMessage.value = ''
  try {
    const resp = await fetch('http://localhost:9012/gameState')
    if (resp.status != 200)
      throw Error('STATUS NOT 200')
    const data = await resp.json()
    console.log(data)
    // if (watcher.checkResponse(data, resp))
    gameStore.updateGameState(data)
  } catch (err) {
    errorMessage.value = 'NO error rescued, but something went wrong !'
    console.warn(err)
  }
}

async function move(event: MouseEvent) {
  const target = event.target as HTMLElement
  const cellId = target.id
  console.log(`Clicked on cell ${cellId}`)
  try {
    const resp = await fetch(`http://localhost:9012/move?id=${cellId}`)
    if (resp.status != 200)
      throw Error('STATUS NOT 200')
    const data = await resp.json()
    console.log(data)
    if (gameStore.backWatcher().checkResponse(data, resp))
      gameStore.updateGameState(data)
  } catch (err) {
    errorMessage.value = 'NO error rescued, but something went wrong !'
    console.warn(err)
  }
}

</script>

<template>
  <section>
    <p class="error" v-if="errorMessage">Message : {{ errorMessage }}</p>
    <!-- Gameboard -->
    <div v-if="gameStore.gameState.board" class="board"
      :style="{
        '--turn-color': gameStore.gameState.board.isBlackToPlay
          ? 'var(--black-color)'
          : 'var(--white-color)'}"
      >
      <div v-for="y in boardDimension" :key="y" class="line">
        <div v-for="x in boardDimension" :key="x" class="cell">
          <div class="circle"
          :class="getCellClass(gameStore.gameState.board?.grid[(x - 1) + (y - 1) * boardDimension] as Cell)"
          :title="`[${x - 1}; ${y - 1}] - id: ${(x - 1) + (y - 1) * boardDimension}`"
          :id="`${(x - 1) + (y - 1) * boardDimension}`"
          @click="move"
            ></div>
        </div>
      </div>
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

      &.black, &.white {
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
      &.black {
        background: var(--black-color);
        color: var(--white-color);
      }
      &.empty {
        opacity: 0;
        transition: all 0.2s ease-in;
      }
      &.empty:hover {
        opacity: 1;
      }
    }
  }

  div.line > div:nth-child(1).cell:nth-child(1) {
    border-bottom: none;
  }

  div.line:nth-child(1) > div.cell {
    border-right: none;
  }
}
</style>
