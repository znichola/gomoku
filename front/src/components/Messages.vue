<script setup lang="ts">
import { computed, nextTick, ref, watch } from 'vue'
import { useGameStore } from '@/stores/game'

const store = useGameStore()
const messages = computed(() => store.gameState.messages)

const scrollEl = ref<HTMLElement | null>(null)
// Jump back to the top (= newest message) whenever the list changes
watch(messages, () => {
  nextTick(() => {
    if (scrollEl.value) scrollEl.value.scrollTop = 0
  })
})
</script>

<template>
  <div class="messages" v-if="messages.length > 0">
    <h2>Messages</h2>
    <div class="scroll" ref="scrollEl">
      <ul>
        <li v-for="(m, i) in messages" :key="i">{{ m }}</li>
      </ul>
    </div>
    <div class="bottom-spacer"></div>
  </div>
</template>

<style scoped lang="less">
h2 {
  padding-bottom: 0.4rem;
  font-size: 1rem;
  text-align: center;
  color: var(--primary-color);
}
.messages {
  display: flex;
  flex-direction: column;
  color: var(--line-color);
  font-family: 'Courier New', Courier, monospace;
  box-sizing: border-box;
  width: 100%;
  max-width: 16rem;
  margin: 0.5rem auto 0;
  height: 100%;
  padding: 0.25rem;
  overflow: auto;

  h2 {
    flex: none;
  }

  .scroll {
    flex: 1;
    min-height: 0;
    overflow-y: auto;
  }

  .bottom-spacer {
    padding: 0.7rem 0;
  }

  ul {
    display: flex;
    // Newest message first, without touching the underlying array: the last
    // (most recent) item just renders at the visual top.
    flex-direction: column-reverse;
    gap: 0.3rem;
  }
  li {
    padding: 0.35rem 0.6rem;
    color: var(--primary-color);
    border: 1px solid var(--line-color);
    white-space: pre-wrap;
    font-size: 0.7rem;
  }
}

// Narrow screens: messages is stacked below the board instead of docked in a
// sidebar, so let it use the full available width, but cap its own height
// (instead of following the page) so it scrolls internally, not endlessly.
@media all and (max-width: 899.98px) and (min-height: 600.02px) {
  .messages {
    max-width: none;
    height: auto;
    max-height: 10rem;
  }
}
</style>
