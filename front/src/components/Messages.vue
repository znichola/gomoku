<script setup lang="ts">
import { computed } from 'vue'
import { useGameStore } from '@/stores/game'

const store = useGameStore()
const messages = computed(() => store.gameState.messages)
</script>

<template>
  <div class="message-container">
    <div class="message" v-for="(m, i) in messages" :key="i">
      {{ m }}
    </div>
  </div>
</template>

<style scoped lang="less">
.message-container {
  border: solid 0.165rem var(--accent-color);
  background-color: var(--bg-color);
  border-radius: 1.2rem;
  padding: 0.4rem 0.6rem;
  color: var(--primary-color);

  display: flex;
  flex-direction: column;

  // max-height: 2rem;
  overflow: hidden;

  transition: max-height 0.25s ease;

  position: fixed;
  top: 2rem;
  right: 2rem;
  z-index: 1000;
}

.message-container:hover {
  // padding: 0.3rem 0;
  max-height: 300px;
}

.message-container:hover {
  animation: enableScroll 0s linear 0.25s forwards;
}

.message {
  padding: 0.25rem 0;
  white-space: pre-wrap;
}

/* To avoid an ugly scroll bar apearing as the div expands */
@keyframes enableScroll {
  to {
    overflow-y: auto;
  }
}
</style>