<script setup lang="ts">
import type { RefStringOrNull } from '@/types/vue'
import { computed, onMounted, onUnmounted, reactive, ref } from 'vue'
import type { Cell, GameState } from '@/types/game'
import { getCellClass } from '@/helpers/helpers'
import { useGameStore } from '@/stores/game'

const errorMessage: RefStringOrNull = ref(null)
const gameStore = useGameStore()

const width = computed(() => gameStore.gameState.board?.width || 19)
const grids = computed(() => {
  const arr: number[][] = []
  for (let y = 0; y < width.value; y++) {
    arr[y] = []
    for (let x = 0; x < width.value; x++) {
      arr[y]![x] = x + y * width.value
    }
  }
  return arr
})

const overlayMap = computed(() => {
  const map = new Map<number, string>()
  for (const m of gameStore.overlayMessages) {
    map.set(m.id, m.msg)
  }
  return map
})


const previewGrid = computed(() => gameStore.watcherState.preview_state)
const iso3D = ref(false)
const overlay = ref(true)

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

let _timeout_delay_ai = 0
async function move(event: MouseEvent) {
  const target = event.target as HTMLElement
  const cellId = target.hasAttribute('id') ? target.id : (target.parentElement as HTMLElement).id
  errorMessage.value = ''
  console.log(`Clicked on cell ${cellId}`)
  let color: string | null = null
  if (gameStore.watcherState.keymode) {
    if (event.shiftKey && !event.ctrlKey) {
      color = 'black'
    } else if (!event.shiftKey && event.ctrlKey) {
      color = 'white'
    }
  }
  try {
    clearTimeout(_timeout_delay_ai)
    const objQuery: { id: string, force_color?: string } = {
      id: cellId.toString()
    }
    if (color)
      objQuery.force_color = color
    const query = new URLSearchParams(objQuery).toString()
    const resp = await fetch(`http://${window.location.hostname}:9012/move?${query}`)
    if (resp.status == 400) {
      errorMessage.value = (await resp.json()).error || 'Invalid move.'
      return
    } else if (resp.status != 200)
      throw Error('STATUS NOT 200')
    const data = await resp.json()
    const updateGS = (data: GameState) => {
      if (gameStore.backWatcher().checkResponse(data, resp))
        gameStore.updateGameState(data)
    }
    if (data.multiple_action) {
      updateGS(data.human)
      if (gameStore.watcherState.speed < 1) {
        updateGS(data.ai)
      } else if (!gameStore.watcherState.enabled) {
        console.log(gameStore.watcherState.speed * 1000)
        _timeout_delay_ai = setTimeout(updateGS, gameStore.watcherState.speed * 1000, data.ai)
      }
    } else {
      if (gameStore.backWatcher().checkResponse(data, resp))
        gameStore.updateGameState(data)
    }
  } catch (err) {
    errorMessage.value = 'Server refused this move.'
    console.warn((err as Error).message)
  }
}

const keyDown = reactive({
  shiftKey: false,
  ctrlKey: false,
  altKey: false
})

onMounted(() => {
  window.addEventListener('keydown', keyMode, true)
  window.addEventListener('keyup', keyMode, true)
})
onUnmounted(() => {
  window.removeEventListener('keydown', keyMode, true)
  window.removeEventListener('keyup', keyMode, true)
})

function keyMode(event: KeyboardEvent) {
  const pressed = (event.type === 'keydown') && gameStore.watcherState.keymode
  switch (event.key) {
    case 'Shift': keyDown.shiftKey = pressed; break
    case 'Control': keyDown.ctrlKey = pressed; break;
    case 'Alt': keyDown.altKey = pressed; break;
    default:
      return
  }
  console.log(event.key, event.type)
  event.stopPropagation()
  event.preventDefault()
}

</script>

<template>
  <section>
    <div id="error-parent">
      <p class="error" v-if="errorMessage" @click="() => errorMessage = null">Message : {{ errorMessage }}</p>
    </div>
    <!-- Gameboard -->
    <div class="board"
      :class="{ iso3D, editMode: keyDown.altKey }"
      :style="{
        '--turn-color': (gameStore.gameState.board?.isBlackToPlay || keyDown.shiftKey) && !keyDown.ctrlKey
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
            :class="{
              highlight: gameStore.highlight.get() == cid,
              overlay: overlay && overlayMap.has(cid)
            }"
            :data-type="getCellClass(gameStore.gameState.board?.grid[cid] as Cell)"
            :title="`[${x}; ${y}] - id: ${cid}`"
            :id="cid.toString()"
            @click="move"
              ><span v-if="overlay">{{ overlayMap.get(cid) }}</span></div>
          </div>
        </div>
      </template>
    </div>
    <button id="isoButton" @click="() => iso3D = !iso3D">iso3D</button>
    <button id="overlayButton" @click="() => overlay = !overlay">overlay</button>
  </section>
</template>

<style scoped lang="less">
div#error-parent {
  position: relative;
}

p.error {
  background: indianred;
  padding: 1.2rem;
  text-align: center;
  color: white;
  transition: all 0.4s ease-in-out;
  opacity: 0.8;
  cursor: pointer;
  &:hover {
    opacity: 0.4;
  }
  position: absolute;
  width: 500px;
  top: 40px;
  z-index: 1000;
  left: 0px;
  right: 0px;
  margin: 0 auto;
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
      z-index: 20;
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
          z-index: 21;
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
      &.overlay {
        opacity: 1;
        display: flex;
        justify-content: center;
        align-items: flex-start;
        z-index: 24;
        span {
          margin: -4px 0;
          color: var(--white-color);
          font-family: sans-serif;
          font-size: calc(var(--celsize) * 0.3);
          display: block;
          width: max-content;
          height: min-content;
          line-height: normal;
          z-index: 40;
          text-shadow: var(--bg-color) 1px 0 0px,
                        var(--bg-color) -1px 0 0px,
                        var(--bg-color) 0 1px 0px,
                        var(--bg-color) 0 -1px 0px;
          pointer-events:none;
        }
        &[data-type=white] span {
          color: var(--black-color);
        }
      }
      &[data-type=empty]:hover {
        opacity: 1;
      }
    }

    &:nth-child(even) div.circle.overlay {
      align-items: flex-end;
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

.editMode div.circle[data-type=white]:hover,
.editMode div.circle[data-type=black]:hover {
  opacity: 0.4;
}

div.board {
  transition:
    .4s ease-in-out transform,
    .4s ease-in-out box-shadow;
}

div.board.iso3D {
  margin-top: calc(var(--celsize) / -2);
  margin-left: calc(var(--celsize) / -2);
  padding-bottom: calc(var(--celsize) / 1);
  padding-right: calc(var(--celsize) / 1);

  transform:
    rotateX(50deg)
    rotateZ(43deg);
  transform-style: flat;
  border-radius: 32px;
  box-shadow:
    1px 1px 0 1px #f9f9fb,
    -1px 0 28px 0 rgba(34, 33, 81, 0.01),
    28px 28px 28px 0 rgba(34, 33, 81, 0.25);
  .circle[data-type=white] {
    box-shadow:
      5px 5px 0px 0px var(--white-color),
      -1px 0 28px 0 rgba(34, 33, 81, 0.01),
      28px 28px 28px 0 rgba(34, 33, 81, 0.25);
  }
  .circle[data-type=black] {
    box-shadow:
      5px 5px 0px 0px var(--black-color),
      -1px 0 28px 0 rgba(34, 33, 81, 0.01),
      28px 28px 28px 0 rgba(34, 33, 81, 0.25);
  }
  div.line div.circle.overlay {
    align-items: center !important;
  }
  div.circle.overlay span {
    transform: rotateZ(-45deg);
  }
  div.cell:nth-child(even) div.circle.overlay span {
    padding-top: 10px;
  }
  div.cell:nth-child(odd) div.circle.overlay span {
    padding-bottom: 10px;
  }
}

#isoButton {
  background: none;
  color: var(--white-color);
  z-index: 1000;
  cursor: pointer;
  position: fixed;
  bottom: 0;
  left: 0;
}

#overlayButton {
  background: none;
  color: var(--white-color);
  z-index: 1000;
  cursor: pointer;
  position: fixed;
  bottom: 0;
  right: 0;
}
</style>
