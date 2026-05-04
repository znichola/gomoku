<script setup lang="ts">
import MiniBoard from '@/components/MiniBoard.vue'
import type { Position } from '@/types/miniBoard'

const props = defineProps<Position>()
</script>

<template>
  <article
    class="section"
    :class="{ 'section--accent': props.accent }"
  >
    <header class="section-head">
      <span
      v-if="!!props.label"
        class="pill"
        :class="{ 'pill--highlight': props.accent }"
      >
        {{ props.label }}
      </span>

      <h2>{{ props.title }}</h2>
      <p v-html="props.description"></p>
      <slot ></slot>
    </header>

    <div class="board-wrap">
      <MiniBoard v-bind="props" />
    </div>
  </article>
</template>

<style scoped>
.section {
  padding: .75rem;
  /* margin: 0 1.25rem; */
  display: flex;
  gap: 1rem;
  flex-wrap: wrap;
  justify-content: space-around;
}

.section--accent {
  border-color: var(--highlight-color);
  background-color: color-mix(in srgb, var(--bg-color) 70%, #1d3a3e 30%);
}

.section-head {
  flex-grow: 1;
}

.section-head h2 {
  font-family: var(--ui-font);
  font-size: 1.5rem;
  font-weight: bold;
  color: var(--primary-color);
  margin: 0.3rem 0 0.3rem;
  letter-spacing: 0.03em;
}

.section-head p {
  font-family: var(--ui-font);
  color: var(--primary-color);
  margin: 0;
  line-height: 1.5;
}

.pill {
  display: inline-block;
  font-family: var(--ui-font);
  font-size: 0.68rem;
  font-weight: bold;
  letter-spacing: 0.08em;
  text-transform: uppercase;
  background: transparent;
  color: var(--accent-color);
  border: 1px solid var(--accent-color);
  padding: 1px 8px;
  border-radius: 2px;
  line-height: normal;
}

.pill--highlight {
  color: var(--highlight-color);
  border-color: var(--highlight-color);
}

.board-wrap {
  display: flex;
  justify-content: center;
  align-items: flex-end;
}

.section-head :deep(ul) {
  font-family: var(--ui-font);
  font-size: 0.78rem;
  color: var(--primary-color);
  margin: 0.4rem 0 0;
  padding-left: 1.2rem;
  line-height: 1.5;
  list-style-type: disc;
}

.section-head :deep(li) {
  margin-bottom: 0.2rem;
}

.section-head :deep(li:last-child) {
  margin-bottom: 0;
}

.section-head :deep(.black) {
  font-weight: bolder;
  color: var(--black-bright-color);
}

.section-head :deep(.white) {
  font-weight: bolder;
  color: var(--white-bright-color);
}

.section-head :deep(strong) {
  color: var(--highlight-color)
}

.section-head :deep(em) {
  font-weight: 600;
}
</style>