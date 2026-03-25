<script setup lang="ts">
import type { RefStringOrNull } from '@/types/vue'
import { onMounted, ref } from 'vue'

const errorMessage: RefStringOrNull = ref(null)
const boardSize = ref(0)

onMounted(load)

async function load() {
  errorMessage.value = "Je ne suis pas content !"
  try {
    const resp = await fetch('http://localhost:9012/gameState')
    console.log(resp.status) // TODO: Improve this
    const data = await resp.json()
    console.log(data)
    boardSize.value = data.board.boardSize
  } catch (err) {
    errorMessage.value = ''
    console.warn(err)
  }
}
</script>

<template>
  <main>
    <p class="error" v-if="errorMessage">Message : {{ errorMessage }}</p>
    <!-- Gameboard -->
    <div class="board">
      <div v-for="y in boardSize" class="line">
        <div v-for="x in boardSize" class="cell">
          <div class="circle" :class="Math.random() < 0.5 ? (Math.random() < 0.5 ? 'black' : 'white') : 'empty'"
          :title="`[${x - 1}; ${y - 1}] - id: ${(x - 1) + (y - 1) * boardSize}`"
            >{{(x - 1) + (y - 1) * boardSize}}</div>
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
  --bg-color: #2d3442;
  background-color: var(--bg-color);
}

div.board {
  --celsize: 40px;
  --line-color: #aab0b0;
  --text-color: var(--bg-color);
  --active-color: aquamarine;
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
