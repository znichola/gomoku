<script setup lang="ts">
import { computed } from 'vue'
import { useGameStore } from '@/stores/game'
import bowlofstones from './BowlOfStones.vue'

const store = useGameStore()
const board = computed(() => store.gameState.board)
const blackCaptured = computed(() => board.value?.blackCaptured ?? 0)
const whiteCaptured = computed(() => board.value?.whiteCaptured ?? 0)
</script>

<template>
  <div class="hud">
    <div class="capture-card black">
      <div class="capture-bowl">
        <bowlofstones />
        <span class="capture-count">{{ blackCaptured }}</span>
      </div>
    </div>

    <div class="capture-card white">
      <div class="capture-bowl">
        <bowlofstones />
        <span class="capture-count">{{ whiteCaptured }}</span>
      </div>
    </div>
  </div>
</template>

<style lang="less">
.hud {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 1rem;
  margin: 0 1rem;
}

.capture-card {
  display: flex;
  flex-direction: column;
  align-items: center;

  &.black {
    color: var(--black-color);
    .capture-count {
      color: var(--white-color);
    }
  }

  &.white {
    color: var(--white-color);
    .capture-count {
      color: var(--black-color);
    }
  }
}

.capture-bowl {
  position: relative;
  display: inline-flex;
  align-items: center;
  justify-content: center;
  width: 3rem;
  height: 3rem;
}

.capture-count {
  position: absolute;
  font: var(--ui-font);
  font-size: 2.2em;
  pointer-events: none;
}
</style>
