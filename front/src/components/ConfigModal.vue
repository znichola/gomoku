<script setup lang="ts">
import { ref, watch, onBeforeUnmount } from 'vue'
import { useGameStore } from '@/stores/game'
import AppButton from '@/components/AppButton.vue'

const props = defineProps<{
  open: boolean
}>()

const emit = defineEmits<{
  (e: 'close'): void
}>()

const gameStore = useGameStore()

const localAI = ref<0 | 1 | 2>(0)
const localSearch = ref<SearchFunction>('ALPHABETA_NEGAMAX_TT')
const localDepth = ref<number>(1)
const localSuggest = ref<MoveSuggestion>('off')


watch(
  () => props.open,
  (open) => {
    if (!open) return

    localAI.value = gameStore.gameState.isAIGame as 0 | 1 | 2
    localSearch.value = gameStore.gameState.searchFunction 
    localDepth.value = gameStore.gameState.searchDepth
    localSuggest.value = gameStore.gameState.moveSuggestion
  }
)

function onKeydown(e: KeyboardEvent) {
  if (e.key === 'Escape' && props.open) {
    emit('close')
  }
}

window.addEventListener('keydown', onKeydown)

onBeforeUnmount(() => {
  window.removeEventListener('keydown', onKeydown)
})

async function applyConfig() {
  try {
    const params = new URLSearchParams({
      isAIGame: String(localAI.value),
      searchFunction: localSearch.value,
      searchDepth: String(localDepth.value),
      moveSuggestion: String(localSuggest.value),
    })
    emit('close')
    const resp = await fetch(
      `http://${window.location.hostname}:9012/set-config?${params}`,
      { method: 'GET' }
    )
    const data = await resp.json()
    if (!resp.ok) {
      console.warn('Config update failed:', resp.status, resp.statusText)
      return
    }
    gameStore.updateGameState(data)
  } catch (err) {
    console.warn(err)
  }
}

function onBackdropClick(e: MouseEvent) {
  if (e.target === e.currentTarget) emit('close')
}

const searchOptions: { value: SearchFunction; label: string; desc: string }[] = [
  { value: 'MINMAX',               label: 'Minimax',         desc: 'Basic two-player search' },
  { value: 'NEGAMAX',              label: 'Negamax',         desc: 'Simplified minimax variant' },
  { value: 'ALPHABETA_NEGAMAX',    label: 'Alpha-beta',      desc: 'Negamax with alpha-beta pruning' },
  { value: 'ALPHABETA_NEGAMAX_TT', label: 'Alpha-beta + TT', desc: 'Negamax with alpha-beta pruning and a transposition table' },
]

const suggestOptions = [
  { value: 'off',   label: 'Off' },
  { value: 'black', label: 'Black' },
  { value: 'white', label: 'White' },
  { value: 'both',  label: 'Both' },
]
</script>

<template>
  <teleport to="body">
    <div v-if="open" class="cm-backdrop" @click="onBackdropClick" aria-hidden="false">
      <div class="cm-modal" role="dialog" aria-modal="true" aria-label="Game configuration">

        <div class="cm-head">
          <h2 class="cm-title">Config</h2>
          <AppButton @click="emit('close')" aria-label="Close" class="cm-close">x</AppButton>
        </div>

        <div class="cm-body">

          <section class="cm-section">
            <h3>AI opponent</h3>
            <em style="margin-bottom: 0.4rem; display: block;">TODO: enabled after centralising AI toggles</em>
            <div class="cm-row">
              <AppButton
                :disabled="true"
                :active="localAI === 1"
                @click="localAI = localAI === 1 ? 0 : 1"
              >Black</AppButton>
              <AppButton
                :disabled="true"
                :active="localAI === 2"
                @click="localAI = localAI === 2 ? 0 : 2"
              >White</AppButton>
            </div>
          </section>

          <section class="cm-section">
            <h3>Search algorithm</h3>
            <div class="cm-row cm-row--wrap">
              <AppButton
                v-for="opt in searchOptions"
                :key="opt.value"
                :active="localSearch === opt.value"
                @click="localSearch = opt.value"
                :title="opt.desc"
              >{{ opt.label }}</AppButton>
            </div>
          </section>

          <section class="cm-section">
            <h3>Search depth</h3>
            <div class="cm-depth-row">
              <input
                type="range"
                min="1"
                max="10"
                step="1"
                v-model.number="localDepth"
              />
              <span class="cm-depth-value">{{ localDepth }}</span>
            </div>
            <p class="cm-hint">
              <template v-if="localDepth <= 2">Puny depth, weak play, but fast response.</template>
              <template v-else-if="localDepth <= 5">Decent effort, yet it remains weak.</template>
              <template v-else-if="localDepth <= 7">Slightly serious, mediocre depth.</template>
              <template v-else-if="localDepth <= 9">Somewhat serious. There is some depth, but it's slower.</template>
              <template v-else>Very strong, maximum depth, expect enlightenment.</template>
            </p>
          </section>

          <section class="cm-section">
            <h3>Move suggestions</h3>
            <em style="margin-bottom: 0.4rem; display: block;">TODO: Implement on back and front</em>
            <div class="cm-row">
              <AppButton
                v-for="opt in suggestOptions"
                :key="opt.value"
                :active="localSuggest === opt.value"
                @click="localSuggest = opt.value"
              >{{ opt.label }}</AppButton>
            </div>
          </section>

        </div>

        <div class="cm-foot">
          <AppButton @click="emit('close')">Cancel</AppButton>
          <AppButton variant="accent" @click="applyConfig">Apply</AppButton>
        </div>

      </div>
    </div>
  </teleport>
</template>

<style scoped lang="less">
.cm-backdrop {
  position: fixed;
  inset: 0;
  background: rgba(0, 0, 0, 0.55);
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 1rem;
  z-index: 9999;
}

.cm-modal {
  width: min(520px, 92vw);
  max-height: 92vh;
  overflow-y: auto;
  border: solid 0.165rem var(--accent-color);
  background-color: var(--bg-color);
  border-radius: 1.2rem;
  color: var(--primary-color);
  box-shadow: 0 20px 60px rgba(0, 0, 0, 0.45);
}

.cm-head {
  position: sticky;
  top: 0;
  z-index: 1;
  background-color: var(--bg-color);
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 1rem;
  padding: 0.8rem 0.9rem 0.6rem;
  border-bottom: solid 1px var(--line-color);

  .cm-close {
    padding: 0.1rem 0.5rem;
    font-size: 1.4rem;
    line-height: 1;
  }
}

.cm-title {
  font-family: var(--title-font-family);
  font-weight: 400;
  font-size: 2.2rem;
  color: var(--accent-color);
  line-height: 1;
}

.cm-body {
  padding: 0.4rem 0.9rem 0.6rem;
}

.cm-section {
  padding: 0.6rem 0;

  & + & {
    border-top: solid 1px var(--line-color);
  }

  h3 {
    font: var(--ui-font);
    font-size: 0.95rem;
    color: var(--line-color);
    margin: 0 0 0.5rem;
    text-transform: uppercase;
    letter-spacing: 0.05em;
  }
}

.cm-row {
  display: flex;
  gap: 0.5rem;

  &--wrap {
    flex-wrap: wrap;
  }
}

.cm-depth-row {
  display: flex;
  align-items: center;
  gap: 0.8rem;

  input[type='range'] {
    flex: 1;
    accent-color: var(--accent-color);
    cursor: pointer;
  }
}

.cm-depth-value {
  font: var(--ui-font);
  color: var(--accent-color);
  min-width: 1.2rem;
  text-align: right;
}

.cm-hint {
  margin: 0.35rem 0 0;
  font-family: var(--ui-font-family);
  font-size: 0.8rem;
  color: var(--line-color);
  line-height: 1.4;
}

.cm-foot {
  position: sticky;
  bottom: 0;
  background-color: var(--bg-color);
  display: flex;
  justify-content: flex-end;
  gap: 0.5rem;
  padding: 0.7rem 0.9rem 0.9rem;
  border-top: solid 1px var(--line-color);
}
</style>