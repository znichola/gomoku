<script setup lang="ts">
import { computed, onMounted, onUnmounted, ref } from 'vue'
import { useGameStore } from '@/stores/game'
import { Cell } from '@/types/game'

const gameStore = useGameStore()

// Live clock driven by rAF while the AI is thinking, so the counter visibly ticks up.
const now = ref(performance.now())
let raf = 0

function tick() {
  now.value = performance.now()
  raf = requestAnimationFrame(tick)
}

onMounted(() => { raf = requestAnimationFrame(tick) })
onUnmounted(() => cancelAnimationFrame(raf))

const liveSeconds = computed(() => Math.max(0, now.value - gameStore.thinking.startedAt) / 1000)

const lastMoveSeconds = computed(() => {
  const micros = gameStore.gameState.aiMoveMicros
  return micros >= 0 ? micros / 1_000_000 : null
})

const seconds = computed(() => gameStore.thinking.active ? liveSeconds.value : lastMoveSeconds.value)
const visible = computed(() => gameStore.gameState.isAIGame !== Cell.EMPTY || gameStore.thinking.active)
</script>

<template>
  <div
    v-if="visible"
    class="ai-timer"
    :class="{ thinking: gameStore.thinking.active }"
    :title="gameStore.thinking.active ? 'AI is thinking…' : 'Time the AI took to find its last move'"
  >
    <span class="dot" aria-hidden="true" />
    <span class="label">AI</span>
    <span class="value">{{ seconds !== null ? seconds.toFixed(2) + 's' : '—' }}</span>
  </div>
</template>

<style scoped lang="less">
.ai-timer {
  display: flex;
  align-items: center;
  gap: 0.35rem;
  padding: 0.2rem 0.6rem;
  border: solid 0.12rem var(--line-color);
  border-radius: 1rem;
  font: var(--ui-font);
  font-size: 0.85rem;
  color: var(--primary-color);
  background-color: var(--bg-color);

  .dot {
    width: 0.5rem;
    height: 0.5rem;
    border-radius: 50%;
    background-color: var(--line-color);
  }

  .label {
    text-transform: uppercase;
    letter-spacing: 0.05em;
    font-size: 0.7rem;
    color: var(--line-color);
  }

  .value {
    font-variant-numeric: tabular-nums;
    min-width: 3.4em;
    text-align: right;
  }

  &.thinking {
    border-color: var(--accent-color);

    .dot {
      background-color: var(--accent-color);
      animation: ai-timer-pulse 0.9s ease-in-out infinite;
    }

    .value {
      color: var(--accent-color);
    }
  }
}

@keyframes ai-timer-pulse {
  0%, 100% { opacity: 1; transform: scale(1); }
  50% { opacity: 0.4; transform: scale(1.3); }
}
</style>
