<script setup lang="ts">
import { computed } from 'vue'
import BoardPositionsSection from '@/components/BoardPositionsSection.vue'
import PositionStats from './PositionStats.vue'
import { positions } from './positions'

function resolvePosition(pos: Position): Position {
  if (!pos.history || pos.history.length === 0) return pos

  const black: number[] = []
  const white: number[] = []
  const highlight: HightLable[] = []

  pos.history.forEach((cell, i) => {
    if (i % 2 === 0) black.push(cell)
    else white.push(cell)
  })

  // TODO : fix to actually display the AI move, another todo elsewhere
  highlight.push({id: pos.history[pos.history.length - 1], highlight: true})

  return { ...pos, black, white, highlight }
}


const resolvedPositions = computed(() => positions.map(resolvePosition))
</script>

<template>
  <main class="tpage">
    <header class="tpage-header">
      <h1 class="tpage-title">Position <span>Playground</span></h1>
      <p class="tpage-sub">An interactive gallery for testing positions, and their backend evaluation</p>
    </header>

    <div class="gallery">
      <BoardPositionsSection
        v-for="pos in resolvedPositions"
        :key="pos.id"
        v-bind="pos"
      >
        <PositionStats 
          :position="pos"
        />
      </BoardPositionsSection>
    </div>
  </main>
</template>

<style scoped>
.tpage {
  min-height: 100vh;
  color: var(--primary-color);
  padding: 2rem 1.5rem 4rem;
  box-sizing: border-box;
  font-family: var(--ui-font-family);
}

.tpage-header {
  text-align: center;
  margin-bottom: 2.5rem;
}

.tpage-title {
  font-family: var(--title-font-family);
  font-size: clamp(2.8rem, 6vw, 5rem);
  font-weight: 400;
  color: var(--accent-color);
  margin: 0 0 0.2rem;
  letter-spacing: 0.02em;
  span {
    font-size: inherit;
  }
}

.tpage-title span {
  color: var(--primary-color);
}

.tpage-sub {
  font-family: var(--ui-font);
  font-size: 0.95rem;
  color: var(--line-color);
  margin: 0;
  letter-spacing: 0.06em;
  text-transform: uppercase;
}

.gallery {
  max-width: 1100px;
  margin: 0 auto;
  display: flex;
  flex-direction: column;
  gap: 1.25rem;
}
</style>