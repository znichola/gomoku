<script setup lang="ts">
import { computed } from 'vue'
import { useGameStore } from '@/stores/game'
import bowlofstones from './BowlOfStones.vue'

const gameStore = useGameStore()
const board = computed(() => gameStore.gameState.board)
const blackCaptured = computed(() => board.value?.blackCaptured ?? 0)
const whiteCaptured = computed(() => board.value?.whiteCaptured ?? 0)

const aiGame = computed(() => gameStore.gameState.isAIGame)
</script>

<template>
  <div class="hud">
    <div class="capture-card black" :class="{ai: aiGame === 1}">
      <div class="capture-bowl">
        <bowlofstones />
        <span class="capture-count">{{ blackCaptured }}</span>
      </div>
    </div>

    <div class="capture-card white" :class="{ai: aiGame === 2}">
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

  position: relative;
  &.ai::after {
    content: 'AI';
    font-size: 0.8rem;
    font-weight: bold;
    position: absolute;
    right: 0;
    bottom: -0.2rem;
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
