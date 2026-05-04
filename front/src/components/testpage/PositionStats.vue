<script setup lang="ts">
import { ref, watch, onMounted, onUnmounted } from 'vue'
import type { Position } from '@/types/miniBoard'
import { usePositionQueueStore } from '@/stores/testPositionQueue'
import router from '@/router'

const props = defineProps<{
  position: Position
}>()

// TODO : also add a button to load the position in a game, like from the main app
// TODO : also make the config assecible from test page, it should also be used when posting to the backend

// TODO : finish push new computer position from backend
const emit = defineEmits<{
  (e: 'update:boardPosition', value: { black: number[], white: number[], lastMove: number }): void
}>()

type Status = 'idle' | 'queued' | 'loading' | 'done' | 'error'

const messages = ref<string[]>([])
const status = ref<Status>('idle')
const queue = usePositionQueueStore()

const IGNORED_PREFIXES = ['{"layer"']
const shouldIgnore = (msg: string) => IGNORED_PREFIXES.some(p => msg.trimStart().startsWith(p))
const whoseTurn = (history: number[]): 1 | 2 => history.length % 2 === 0 ? 1 : 2

let alive = true
onUnmounted(() => { alive = false })

  const get = (path: string) => fetch(`http://${window.location.hostname}:9012${path}`).then(r => {
    if (!r.ok) throw new Error(`${path} failed: ${r.status}`)
    return r
  })

function scheduleStats() {
  status.value = 'queued'
  messages.value = []

  queue.enqueue(async () => {
    if (!alive) return

    status.value = 'loading'
    const history = props.position.history ?? []

    try {
      if (!history.length) throw new Error('History is empty, no position to load')

      await get('/set-config?isAIGame=0')
      await get('/reset')
      await get(`/replay?moves=${history.join(',')}`)

      const aiPlayer = whoseTurn(history)
      const data: GameState = await get(`/set-config?isAIGame=${aiPlayer}`).then(r => r.json())
      console.log(aiPlayer, 'DATA', data)

      await get('/set-config?isAIGame=0')

      if (!alive) return
      messages.value = data.messages.filter(m => !shouldIgnore(m))
      
      // Convert grid to black and white positions (1 = black, 2 = white, 0 = empty)
      const black: number[] = []
      const white: number[] = []
      const grid = data.board.grid as number[]
      grid.forEach((cell, index) => {
        if (cell === 1) black.push(index)
        else if (cell === 2) white.push(index)
      })
      const lastMove: number = data.moveHistory[data.moveHistory.length - 1]

      console.log("Black", black, "White", white, "lastMove", lastMove);
      
      emit('update:boardPosition', { black, white, lastMove })
      status.value = 'done'
    } catch (err: any) {
      if (!alive) return
      messages.value = [String(err?.message ?? err)]
      status.value = 'error'
    }
  })
}

async function loadPosition() {
  const history = props.position.history ?? []
  try {
    await get('/set-config?isAIGame=0')
    await get('/reset')
    await get(`/replay?moves=${history.join(',')}`)

    router.push('/');
  } catch (err: any) {
    console.log('Error loading position: ', err?.message ?? err)
  }
}

watch(() => props.position.id, scheduleStats)
onMounted(scheduleStats)
</script>

<template>
  <div class="root">
    <div v-if="status === 'queued'" class="block">
      <span class="spinner spinner--queued" aria-hidden="true" />
      <span>Waiting in queue…</span>
    </div>

    <div v-else-if="status === 'loading'" class="block">
      <span class="spinner" aria-hidden="true" />
      <span>Querying backend…</span>
    </div>

    <div v-else-if="messages.length" class="block">
      <pre
        v-for="(msg, i) in messages"
        :key="i"
        class="line"
        :class="{
          'line--eval':     msg.startsWith('Evaluate'),
          'line--ai':       msg.startsWith('[AI]'),
          'line--cache':    msg.startsWith('[CACHE]'),
          'line--ram':      msg.startsWith('[RAM]'),
          'line--thinking': msg.includes('AI is thinking'),
          'line--error':    status === 'error',
        }"
      >{{ msg.trimEnd() }}</pre>
      
      <div class="ctrl-pos ctrl-spacing">
        <button
          class="ctrl-btn"
          :disabled="status === 'queued' || status === 'loading'"
          title="Refetch"
          @click="scheduleStats"
        >↺</button>
        <button
          class="ctrl-btn"
          :disabled="status === 'queued' || status === 'loading'"
          title="Load"
          @click="loadPosition"
        >↪</button>
      </div>
    </div>

    <div v-else-if="status === 'done'" class="empty">
      No output from backend.
      <button class="ctrl-pos ctrl-btn ctrl-btn--inline" @click="scheduleStats">↺ Retry</button>
    </div>

    <div v-else-if="status === 'error'" class="empty empty--error">
      <button class="ctrl-pos ctrl-btn ctrl-btn--inline" @click="scheduleStats">↺ Retry</button>
    </div>

  </div>
</template>

<style scoped lang="less">
.root {
  font-family: var(--mono-font-family, 'Fira Mono', 'Consolas', monospace);
  font-size: 0.78rem;
  line-height: 1.55;
  position: relative;
}

.loading {
  display: flex;
  align-items: center;
  gap: 0.55rem;
  color: var(--line-color);
  padding: 0.4rem 0;
  height: 200px;
}

.spinner {
  display: inline-block;
  width: 0.85rem;
  height: 0.85rem;
  border: 2px solid var(--line-color);
  border-top-color: var(--accent-color);
  border-radius: 50%;
  animation: spin 0.8s linear infinite;
  flex-shrink: 0;

  &--queued {
    border-top-color: var(--line-color);
    opacity: 0.45;
    animation-duration: 1.6s;
  }
}

@keyframes spin {
  to { transform: rotate(360deg); }
}

.block {
  background: color-mix(in srgb, var(--bg-color) 60%, #000 40%);
  border: 1px solid var(--line-color);
  border-radius: 0.55rem;
  padding: 0.65rem 0.85rem;
  overflow-x: auto;
  height: 200px;
}

.line {
  margin: 0;
  white-space: pre-wrap;
  word-break: break-word;
  color: var(--primary-color);
  font-size: small;

  & + & {
    margin-top: 0.45rem;
    padding-top: 0.45rem;
    border-top: 1px solid color-mix(in srgb, var(--line-color) 30%, transparent 70%);
  }

  &--eval {
    color: var(--accent-color);
    font-weight: 600;
  }

  &--ai {
    color: color-mix(in srgb, var(--primary-color) 70%, var(--accent-color) 30%);
  }

  &--cache,
  &--ram {
    opacity: 0.5;
    font-size: 0.72rem;
  }

  &--thinking {
    font-style: italic;
    opacity: 0.65;
  }

  &--error {
    color: #e05c5c;
  }
}

.empty {
  color: var(--line-color);
  font-style: italic;
  font-size: 0.8rem;
  display: flex;
  align-items: center;
  gap: 0.5rem;

  &--error {
    color: #e05c5c;
  }
}

.ctrl-pos {
  position: absolute;
  top: 0.6rem;
  right: 0.8rem;
  font-size: initial;
}

.ctrl-spacing .ctrl-btn:first-child {
  margin-right: 0.2rem;
}

.ctrl-btn {
  background: none;
  border: 1px solid var(--line-color);
  border-radius: 0.3rem;
  color: var(--line-color);
  cursor: pointer;
  font-size: 0.85rem;
  padding: 0.1rem 0.35rem;
  line-height: 1;
  // opacity: 0;
  transition: opacity 0.15s, color 0.15s;

  .block:hover & {
    opacity: 1;
  }

  &:hover {
    color: var(--accent-color);
    border-color: var(--accent-color);
  }

  &:disabled {
    opacity: 0.3 !important;
    cursor: not-allowed;
  }

  &--inline {
    position: static;
    opacity: 1;
    font-style: normal;
  }
}

// scroll bar
.block {
  scrollbar-width: thin;
  scrollbar-color: rgba(0, 0, 0, 0.35) transparent;
}

.block::-webkit-scrollbar {
  height: 6px;
  width: 6px;
}

.block::-webkit-scrollbar-thumb {
  background: rgba(0, 0, 0, 0.25);
  border-radius: 10px;
}

.block::-webkit-scrollbar-track {
  background: transparent;
}
</style>