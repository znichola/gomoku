<script setup lang="ts">
import { ref, watch, onUnmounted, computed, nextTick } from 'vue'
import { Cell } from '@/types/game'
import { useGameStore } from '@/stores/game'

const winner = computed(() => gameStore.gameState.board?.winner || Cell.EMPTY)

const emit = defineEmits<{
  (e: 'close'): void
}>()

const gameStore = useGameStore()
const canvasEl = ref<HTMLCanvasElement | null>(null)

interface Particle {
  x: number
  y: number
  vx: number
  vy: number
  targetX: number
  targetY: number
  color: string
  radius: number
}

let animFrame = 0
let particles: Particle[] = []
let frameCount = 0
const EXPLODE_DURATION = 80

const open = computed(() => winner.value !== Cell.EMPTY)

const playerType = computed(() => {
  if (winner.value === Cell.EMPTY) return ''
  const isAIGame = gameStore.gameState.isAIGame
  if (winner.value === Cell.BLACK && isAIGame === Cell.BLACK) return 'AI'
  if (winner.value === Cell.WHITE && isAIGame === Cell.WHITE) return 'AI'
  return 'H'
})

const winnerLabel = computed(() => {
  if (winner.value === Cell.BLACK) return 'Black'
  if (winner.value === Cell.WHITE) return 'White'
  return 'Draw'
})

const winnerColor = computed(() => {
  if (winner.value === Cell.BLACK) return '#d35013'
  if (winner.value === Cell.WHITE) return '#e2d9b2'
  return '#6dbfcb'
})

function getPiecePositions(): { x: number; y: number; color: string }[] {
  const pieces: { x: number; y: number; color: string }[] = []
  document
    .querySelectorAll<HTMLElement>('.circle[data-type=black], .circle[data-type=white]')
    .forEach((el) => {
      const rect = el.getBoundingClientRect()
      const cx = rect.left + rect.width / 2
      const cy = rect.top + rect.height / 2
      const color = el.getAttribute('data-type') === 'black' ? '#d35013' : '#e2d9b2'
      pieces.push({ x: cx, y: cy, color })
    })
  return pieces
}

function getTextTargets(
  canvasWidth: number,
  canvasHeight: number,
  text: string,
): { x: number; y: number }[] {
  const offscreen = document.createElement('canvas')
  offscreen.width = canvasWidth
  offscreen.height = canvasHeight
  const ctx = offscreen.getContext('2d')!

  ctx.fillStyle = '#000000'
  ctx.fillRect(0, 0, canvasWidth, canvasHeight)

  const fontSize = Math.min(canvasWidth / (text.length * 0.65), 160)
  ctx.font = `bold ${fontSize}px 'Pirata One', Impact, 'Arial Narrow Bold', sans-serif`
  ctx.textAlign = 'center'
  ctx.textBaseline = 'middle'
  ctx.fillStyle = '#ffffff'
  ctx.fillText(text, canvasWidth / 2, canvasHeight * 0.42)

  const imageData = ctx.getImageData(0, 0, canvasWidth, canvasHeight)
  const data = imageData.data
  const targets: { x: number; y: number }[] = []

  const step = 7
  for (let y = 0; y < canvasHeight; y += step) {
    for (let x = 0; x < canvasWidth; x += step) {
      const idx = (y * canvasWidth + x) * 4
      if ((data[idx] ?? 0) > 128) {
        targets.push({ x, y })
      }
    }
  }

  return targets
}

async function startAnimation() {
  if (!canvasEl.value || !winner) return

  const canvas = canvasEl.value
  canvas.width = window.innerWidth
  canvas.height = window.innerHeight

  await document.fonts.load(`bold 160px 'Pirata One'`).catch(() => {})

  const piecePositions = getPiecePositions()
  const targets = getTextTargets(canvas.width, canvas.height, winner.value === Cell.BLACK ? 'Black' : 'White')

  // Fisher-Yates shuffle for uniform distribution of particles across text
  for (let i = targets.length - 1; i > 0; i--) {
    const j = Math.floor(Math.random() * (i + 1))
    ;[targets[i], targets[j]] = [targets[j]!, targets[i]!]
  }

  const screenCenterX = canvas.width / 2
  const screenCenterY = canvas.height / 2

  // Build source list – duplicate board pieces to fill all text targets
  const sources = [...piecePositions]
  const fallbackColor =
    winner.value === Cell.BLACK
      ? '#d35013'
      : winner.value === Cell.WHITE
        ? '#e2d9b2'
        : '#6dbfcb'
  while (sources.length < targets.length) {
    if (piecePositions.length === 0) {
      sources.push({
        x: screenCenterX + (Math.random() - 0.5) * 300,
        y: screenCenterY + (Math.random() - 0.5) * 300,
        color: fallbackColor,
      })
    } else {
      sources.push(piecePositions[Math.floor(Math.random() * piecePositions.length)]!)
    }
  }

  particles = []
  for (let i = 0; i < targets.length; i++) {
    const src = sources[i % sources.length]!
    const tgt = targets[i]!

    const dx = src.x - screenCenterX
    const dy = src.y - screenCenterY
    const len = Math.sqrt(dx * dx + dy * dy) || 1
    const speed = 8 + Math.random() * 14

    // For DRAW keep each piece's own color; for wins force winner color
    let particleColor: string
    if (winner.value === Cell.OUTSIDE) {
      particleColor = src.color
    } else {
      particleColor = fallbackColor
    }

    particles.push({
      x: src.x,
      y: src.y,
      vx: (dx / len) * speed + (Math.random() - 0.5) * 8,
      vy: (dy / len) * speed + (Math.random() - 0.5) * 8,
      targetX: tgt.x,
      targetY: tgt.y,
      color: particleColor,
      radius: 5,
    })
  }

  frameCount = 0
  cancelAnimationFrame(animFrame)
  animate()
}

function animate() {
  const canvas = canvasEl.value
  if (!canvas) return
  const ctx = canvas.getContext('2d')!

  // Semi-transparent fill creates motion trail and dims the backdrop gradually
  ctx.fillStyle = 'rgba(45, 52, 66, 0.18)'
  ctx.fillRect(0, 0, canvas.width, canvas.height)

  frameCount++
  const exploding = frameCount <= EXPLODE_DURATION

  for (const p of particles) {
    if (exploding) {
      p.vy += 0.38
      p.vx *= 0.985
      p.vy *= 0.985
    } else {
      const dx = p.targetX - p.x
      const dy = p.targetY - p.y
      p.vx += dx * 0.055
      p.vy += dy * 0.055
      p.vx *= 0.86
      p.vy *= 0.86
    }

    p.x += p.vx
    p.y += p.vy

    ctx.beginPath()
    ctx.arc(p.x, p.y, p.radius, 0, Math.PI * 2)
    ctx.fillStyle = p.color
    ctx.fill()
  }

  animFrame = requestAnimationFrame(animate)
}

watch(open, async (newVal) => {
  if (newVal) {
    await nextTick()
    startAnimation()
  } else {
    cancelAnimationFrame(animFrame)
    particles = []
  }
})

onUnmounted(() => {
  cancelAnimationFrame(animFrame)
})

function onBackdropClick(e: MouseEvent) {
  if (e.target === e.currentTarget) emit('close')
}
</script>

<template>
  <teleport to="body">
    <div v-if="open" class="wm-backdrop" @click="onBackdropClick" aria-hidden="false">
      <canvas ref="canvasEl" class="wm-canvas" aria-hidden="true"></canvas>

      <div class="wm-card" role="dialog" aria-modal="true" aria-label="Game over">
        <div class="wm-title">The winner is</div>
        <div class="wm-winner" :style="{ color: winnerColor }">{{ winnerLabel }}!</div>
        <div class="wm-player" v-if="playerType">
          <span class="wm-badge">{{ playerType === 'AI' ? '🤖 AI' : '👤 Human' }}</span>
        </div>
        <button class="wm-close-btn" type="button" @click="emit('close')">Close</button>
      </div>
    </div>
  </teleport>
</template>

<style scoped lang="less">
.wm-backdrop {
  position: fixed;
  inset: 0;
  display: flex;
  align-items: flex-end;
  justify-content: center;
  padding-bottom: 2.5rem;
  z-index: 9999;
}

.wm-canvas {
  position: absolute;
  inset: 0;
  width: 100%;
  height: 100%;
  pointer-events: none;
}

.wm-card {
  position: relative;
  z-index: 1;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 0.5rem;
  background-color: var(--bg-color);
  border: solid 0.165rem var(--accent-color);
  border-radius: 1.2rem;
  padding: 1.2rem 2rem 1rem;
  box-shadow: 0 20px 60px rgba(0, 0, 0, 0.6);
  min-width: 220px;
}

.wm-title {
  font-family: var(--ui-font-family);
  font-size: 1.1rem;
  color: var(--primary-color);
  opacity: 0.8;
  letter-spacing: 0.05em;
}

.wm-winner {
  font-family: var(--title-font-family);
  font-size: 3.5rem;
  font-weight: 400;
  line-height: 1;
}

.wm-player {
  margin-top: 0.1rem;
}

.wm-badge {
  font: var(--ui-font);
  font-size: 1rem;
  color: var(--primary-color);
  border: solid 1px var(--line-color);
  border-radius: 2rem;
  padding: 0.15rem 0.7rem;
}

.wm-close-btn {
  margin-top: 0.6rem;
  border: solid 0.165rem var(--accent-color);
  background-color: transparent;
  color: var(--primary-color);
  border-radius: 0.9rem;
  padding: 0.35rem 1.2rem;
  font: var(--ui-font);
  cursor: pointer;
  transition: background-color 0.2s ease;

  &:hover {
    background-color: rgba(211, 80, 19, 0.15);
  }

  &:active {
    transform: scale(0.96);
  }
}
</style>
