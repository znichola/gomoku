<script setup lang="ts">
import type { RefStringOrNull } from '@/types/vue'
import { onMounted, ref } from 'vue'
import type { Cell } from '@/types/game'
import { getCellClass } from '@/helpers/helpers'
import { useGameStore } from '@/stores/game'

const errorMessage: RefStringOrNull = ref(null)
const gameStore = useGameStore()

const boardDimentions = ref(19)

onMounted(load)

async function load() {
  errorMessage.value = ''
  try {
    const resp = await fetch('http://localhost:9012/gameState')
    console.log(resp.status) // TODO: Improve this
    const data = await resp.json()
    console.log(data)
    gameStore.updateGameState(data);
    boardDimentions.value = data.boardDimentions
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
    const resp = await fetch(`http://localhost:9012/move?id=${cellId}`, {
      method: 'GET',
    })
    const data = await resp.json()
    console.log(data)
    gameStore.updateGameState(data)
  } catch (err) {
    errorMessage.value = 'NO error rescued, but something went wrong !'
    console.warn(err)
  }
}

</script>

<template>
  <content>
    <p class="error" v-if="errorMessage">Message : {{ errorMessage }}</p>
    <!-- Gameboard -->
    <div v-if="gameStore.gameState.board" class="board"
      :style="{
        '--turn-color': gameStore.gameState.board.isBlackToPlay
          ? 'var(--black-color)'
          : 'var(--white-color)'
      }">
      <div v-for="y in boardDimentions" :key="y" class="line">
        <div v-for="x in boardDimentions" :key="x" class="cell">
          <div class="circle"
          :class="getCellClass(gameStore.gameState.board?.grid[(x - 1) + (y - 1) * boardDimentions] as Cell)"
          :title="`[${x - 1}; ${y - 1}] - id: ${(x - 1) + (y - 1) * boardDimentions}`"
          :id="`${(x - 1) + (y - 1) * boardDimentions}`"
          @click="move"
            ></div>
        </div>
      </div>
    </div>
  </content>
</template>

<style scoped lang="less">
p.error {
    background: indianred;
    padding: 1.2rem;
    text-align: center;
    color: white;
}

div.board {
  --celsize: min(40px, calc(100vw / 19));
  margin-bottom: var(--celsize);
  margin-right: var(--celsize);

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
