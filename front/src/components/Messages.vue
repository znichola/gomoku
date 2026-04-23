<script setup lang="ts">
import { computed, onMounted, onUnmounted, reactive, ref } from 'vue'
import { useGameStore } from '@/stores/game'
import type { Ref } from 'vue'

const store = useGameStore()
const container: Ref<HTMLElement | null, HTMLElement | null> = ref(null)
const position = reactive({ left: false, top: false });
const messages = computed(() => store.gameState.messages)

onMounted(() => window.addEventListener('mousemove', mousemove, true))
onUnmounted(() => window.removeEventListener('mousemove', mousemove, true))

function mousemove(event: MouseEvent) {
  if (messages.value.length <= 0)
    return ;
  const width = (container.value?.offsetWidth || 0) + 100
  const height = (container.value?.offsetHeight || 0) + 100
  const pageWidth = window.innerWidth
  const pageHeight = window.innerHeight

  //position.left = false
  position.top = (pageWidth - width < event.clientX) && (pageHeight - height < event.clientY)
}
</script>

<template>
  <div id="message-container" :class="position" v-if="messages.length > 0" ref="container">
    <div class="message" v-for="(m, i) in messages" :key="i">
      {{ m }}
    </div>
  </div>
</template>

<style scoped lang="less">
#message-container {
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
  bottom: 2rem;
  right: 2rem;
  z-index: 1000;

  &.left {
    right: unset;
    left: 2rem;
  }

  &.top {
    bottom: unset;
    top: 2rem;
  }
}

#message-container:hover {
  // padding: 0.3rem 0;
  max-height: 300px;
}

#message-container:hover {
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
