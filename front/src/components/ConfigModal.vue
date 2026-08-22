<script setup lang="ts">
import { ref, watch, onBeforeUnmount } from 'vue'
import { useGameStore } from '@/stores/game'
import AppButton from '@/components/AppButton.vue'
import { computed } from 'vue';
import { searchOptions, moveOptions } from '@/helpers/helpers';
import { Cell } from '@/types/game';
import type { MoveFunction, MoveSuggestion, SearchFunction } from '@/types/game';

const props = defineProps<{
  open: boolean
}>()

const emit = defineEmits<{
  (e: 'close'): void
}>()

async function toggleAI(color: 1 | 2) {
  try {
    const v = gameStore.gameState.isAIGame == color ? 0 : color
    gameStore.startThinking()
    const params = new URLSearchParams({ isAIGame: String(v) })
    if (isSuggestOptionDisabled(localSuggest.value, v)) {
      localSuggest.value = 'off'
      params.set('moveSuggestion', 'off')
    }
    const resp = await fetch(`http://${window.location.hostname}:9012/set-config?${params}`, {
      method: 'GET',
    })
    const data = await resp.json()
    gameStore.updateGameState(data);
    } catch (err) {
    console.warn(err)
  }
  gameStore.stopThinking()
}

const gameStore = useGameStore()

const aiGame = computed(() => gameStore.gameState.isAIGame)

function isSuggestOptionDisabled(value: MoveSuggestion, aiColor: Cell = aiGame.value): boolean {
  if (aiColor === Cell.EMPTY || value === 'off') return false
  const humanColor: MoveSuggestion = aiColor === Cell.BLACK ? 'white' : 'black'
  return value !== humanColor
}
const localSearch = ref<SearchFunction>('ALPHABETA_NEGAMAX_TT')
const localMove = ref<MoveFunction>('CANDIDATE_MOVES')
const localDepth = ref<number>(1)
const localTimeBudgetMs = ref<number>(500)
const localSuggest = ref<MoveSuggestion>('off')


watch(
  () => props.open,
  (open) => {
    if (!open) return

    localSearch.value = gameStore.gameState.searchFunction
    localMove.value = gameStore.gameState.moveFunction
    localDepth.value = gameStore.gameState.searchDepth
    localTimeBudgetMs.value = gameStore.gameState.aiTimeBudgetMs
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
      searchFunction: localSearch.value,
      moveFunction: localMove.value,
      searchDepth: String(localDepth.value),
      aiTimeBudgetMs: String(localTimeBudgetMs.value),
      moveSuggestion: String(localSuggest.value),
    })
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

const suggestOptions: { value: MoveSuggestion, label: string }[] = [
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
            <div class="cm-row">
              <AppButton
                :active="aiGame === 1"
                @click="toggleAI(1)"
              >Black</AppButton>
              <AppButton
                :active="aiGame === 2"
                @click="toggleAI(2)"
              >White</AppButton>
            </div>
          </section>

          <section class="cm-section">
            <h3>Move suggestions</h3>
            <div class="cm-row">
              <AppButton
                v-for="opt in suggestOptions"
                :key="opt.value"
                :active="localSuggest === opt.value"
                :disabled="isSuggestOptionDisabled(opt.value)"
                @click="localSuggest = opt.value; applyConfig()"
              >{{ opt.label }}</AppButton>
            </div>
            <p class="cm-hint"> Highlights the AI's suggested move in blue.</p>
          </section>

          <section class="cm-section">
            <h3>Search algorithm</h3>
            <div class="cm-row cm-row--wrap">
              <AppButton
                v-for="opt in searchOptions"
                :key="opt.value"
                :active="localSearch === opt.value"
                @click="localSearch = opt.value; applyConfig()"
                :title="opt.desc"
              >{{ opt.label }}</AppButton>
            </div>
            <p class="cm-hint">
              {{ searchOptions.find(opt => opt.value === localSearch)?.desc }}
            </p>
          </section>

          <section class="cm-section">
            <h3>Move algorithm</h3>
            <div class="cm-row cm-row--wrap">
              <AppButton
                v-for="opt in moveOptions"
                :key="opt.value"
                :active="localMove === opt.value"
                @click="localMove = opt.value; applyConfig()"
                :title="opt.desc"
              >{{ opt.label }}</AppButton>
            </div>
            <p class="cm-hint">
              {{ moveOptions.find(opt => opt.value === localMove)?.desc }}
            </p>
          </section>

          <section class="cm-section">
            <h3>Search depth</h3>
            <div class="cm-range-row">
              <input
                type="range"
                min="1"
                max="10"
                step="1"
                v-model.number="localDepth"
                @change="applyConfig"
              />
              <span class="cm-value">{{ localDepth }}</span>
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
            <h3>AI time budget</h3>
            <div class="cm-range-row">
              <input
                id="aiTimeBudget"
                type="range"
                min="100"
                max="10000"
                step="100"
                v-model.number="localTimeBudgetMs"
                @change="applyConfig"
              />
              <label for="aiTimeBudget"><span class="cm-value">{{ (localTimeBudgetMs / 1000).toFixed(1) }}s</span></label>
            </div>
            <p class="cm-hint">
              Cutoff given to the AI's search: it stops thinking and plays the best move found so far, however deep it got (capped by search depth above).
            </p>
          </section>
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
  overflow-y: auto;
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

.cm-range-row {
  display: flex;
  align-items: center;
  width: 100%;
  gap: 0.8rem;

  input[type='range'] {
    flex: 1;
    accent-color: var(--accent-color);
    cursor: pointer;
  }
}

.cm-value {
  font: var(--ui-font);
  color: var(--accent-color);
  min-width: 1.2rem;
  text-align: right;
}

.cm-hint, *[data-hint]::after {
  margin: 0.35rem 0 0;
  font-family: var(--ui-font-family);
  font-size: 0.8rem;
  color: var(--line-color);
  line-height: 1.4;
}

*[data-hint] {
  position: relative;
  padding-bottom: 1rem;
}

*[data-hint]::after  {
  content: attr(data-hint);
  position: absolute;
  bottom: 0.1rem;
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
