<script setup lang="ts">

import { computed } from 'vue'
import type { Position } from '@/types/miniBoard'

const props = withDefaults(defineProps<Position>(), {
  width: 19,
  height: undefined,
  black: () => [],
  white: () => [],
  highlight: () => [],
  boardHeight: '230px',
})

const cHeight = computed(() => props.height ?? props.width)

const cellSize = computed(() =>
  `calc(${props.boardHeight} / (${cHeight.value} + 1))`
)

const grid = computed(() => {
  const rows: number[][] = []
  for (let y = 0; y < cHeight.value + 1; y++) {
    rows[y] = []
    for (let x = 0; x < props.width +1; x++) {
      rows[y]![x] = x + y * props.width
    }
  }
  return rows
})

const blackSet = computed(() => new Set(props.black))
const whiteSet = computed(() => new Set(props.white))
const highlightSet = computed(() => new Set(props.highlight.map(h => h.id)))

function stoneType(id: number): 'black' | 'white' | 'empty' {
  if (blackSet.value.has(id)) return 'black'
  if (whiteSet.value.has(id)) return 'white'
  return 'empty'
}

function highlightLabel(id: number): string | undefined {
  return props.highlight.find(h => h.id === id)?.label
}

function highlightCircle(id: number): bool {
  return props.highlight.find(h => h.id === id)?.highlight
}
</script>

<template>
  <div
    class="bd-board"
    :style="{
      '--bd-cell': cellSize,
    }"
  >
    <div v-for="(row, y) in grid" :key="y" class="bd-line">
      <div v-for="id in row" :key="id" class="bd-cell">
        <div
          v-if="stoneType(id) !== 'empty' || highlightSet.has(id) "
          class="bd-stone"
          :class="{
            'bd-stone--black':     stoneType(id) === 'black',
            'bd-stone--white':     stoneType(id) === 'white',
            'bd-stone--highlight': !!highlightCircle(id),
          }"
        >{{ highlightLabel(id) }}</div>
      </div>
    </div>
  </div>
</template>

<style scoped>
.bd-board {
  --bd-bg:        var(--bg-color);
  --bd-line:      var(--line-color);
  --bd-black:     var(--black-color);
  --bd-white:     var(--white-color);
  --bd-highlight: var(--highlight-color);

  display: inline-block;
  border: solid 1px  var(--bd-line);
  border-radius: 6px;
  background-color: col();
  padding: calc(var(--bd-cell) / 2);

  box-sizing: content-box;
}

.bd-line {
  display: flex;
}

.bd-cell {
  width:  var(--bd-cell);
  height: var(--bd-cell);
  border-right:  1px solid var(--bd-line);
  border-bottom: 1px solid var(--bd-line);
  position: relative;
  box-sizing: border-box;
}

.bd-line > .bd-cell:last-child { border-right:  none; }
.bd-line:last-child > .bd-cell { border-bottom: none; }

/* ── Stone base ── */
.bd-stone {
  --radius: calc(var(--bd-cell) * 0.78);

  position: absolute;
  left:   calc(var(--bd-cell) - var(--radius) / 2);
  top:    calc(var(--bd-cell) - var(--radius) / 2);
  width:  var(--radius);
  height: var(--radius);
  border-radius: 50%;
  box-sizing: border-box;
  pointer-events: none;
  transition: opacity 0.15s ease;
  z-index: 1;

  
  font-size: clamp(0.7rem, calc(var(--bd-cell) * 0.8 ), 1.2rem) ;
  user-select: none;
  font-weight: 700;
  display: flex;
  align-items: center;
  justify-content: center;
}

.bd-stone--black {
  background: var(--black-color);
  color: var(--white-color);
}

.bd-stone--white {
  background: var(--white-color);
  color: var(--black-color);
}

.bd-stone--highlight::after {
  content: '';
  position: absolute;
  inset: -4px;
  border-radius: 50%;
  border: 2px solid var(--bd-highlight);
}

</style>