<script setup lang="ts">
import type { RefStringOrNull } from '@/types/vue'
import { onMounted, ref } from 'vue'
import type { GameState } from '@/types/game'
import { getCellClass } from '@/helpers/helpers'

const errorMessage: RefStringOrNull = ref(null)
const gameState = ref<GameState | null>(null)

onMounted(load)

async function load() {
  errorMessage.value = ''
  try {
    const resp = await fetch('http://localhost:9012/gameState')
    console.log(resp.status) // TODO: Improve this
    const data = await resp.json()
    console.log(data)
    gameState.value = data
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
    gameState.value = data
  } catch (err) {
    errorMessage.value = 'NO error rescued, but something went wrong !'
    console.warn(err)
  }
}

</script>

<template>
  <main>
    <p class="error" v-if="errorMessage">Message : {{ errorMessage }}</p>
    <!-- Gameboard -->
    <div v-if="gameState" class="board">
      <div v-for="y in gameState.board.boardDimentions" class="line">
        <div v-for="x in gameState.board.boardDimentions" class="cell">
          <div class="circle"
          :class="getCellClass(gameState.board.grid[(x - 1) + (y - 1) * gameState.board.boardDimentions])"
          :title="`[${x - 1}; ${y - 1}] - id: ${(x - 1) + (y - 1) * gameState.board.boardDimentions}`"
          :id="`${(x - 1) + (y - 1) * gameState.board.boardDimentions}`"
          @click="move"
            ></div>
        </div>
      </div>
    </div>
  </main>
</template>

<style scoped lang="less">
p.error {
    background: indianred;
    padding: 20px;
    text-align: center;
    color: white;
}

main {
  width: 100vw;
  min-height: 100vh;
  overflow: hidden;
  display: flex;
  align-items: center;
  justify-content: center;
  --bg-color: #2d3442;
  background-color: var(--bg-color);
}

div.board {
  --celsize: 40px;
  --line-color: #aab0b0;
  --text-color: var(--bg-color);
  --active-color: #d35013;
  margin-bottom: var(--celsize);

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
      --radius: calc(var(--celsize) / 2);
      position: absolute;
      left: calc(var(--celsize) - var(--radius) / 2);
      top: calc(var(--celsize) - var(--radius) / 2);
      width: var(--radius);
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
        background-color: var(--active-color);
      }

      &.black, &.white {
        --radius: calc(var(--celsize) * 0.75);

        background: white;
        &::after {
          position: absolute;
          left: 0px;
          top: 0px;
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
        background: black;
        color: white;
      }
      &.empty {
        display: none;
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
