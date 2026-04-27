<script setup lang="ts">
import { ref, watch, onUnmounted, computed, nextTick, onUpdated, onMounted } from 'vue'
import { Cell } from '@/types/game'
import { useGameStore } from '@/stores/game'

const winner = computed(() => gameStore.gameState.board?.winner || Cell.EMPTY)

const emit = defineEmits<{
  (e: 'close'): void
}>()

const gameStore = useGameStore()
const canvasEl = ref<HTMLCanvasElement | null>(null)

const cardVisible = ref(false)

interface Particle {
  x: number
  y: number
  vx: number
  vy: number
  targetX: number
  targetY: number
  color: string
  radius: number
  burnFrames: number
}

let animFrame = 0
let particles: Particle[] = []
let frameCount = 0
const EXPLODE_DURATION = 80

const isClosing = ref(false)
let closingFrameCount = 0
const CLOSE_DURATION = 55 // ~0.9 s at 60 fps

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
  ctx.font = `400 ${fontSize}px 'Pirata One', Impact, 'Arial Narrow Bold', sans-serif`
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
  if (!canvasEl.value || !winner.value) return

  gameStore.fetchIsAvailable.set(false)
  await new Promise(resolve => setTimeout(resolve, 1000))
  cardVisible.value = true
  await new Promise(resolve => setTimeout(resolve, 1000))
  gameStore.fetchIsAvailable.set(true)

  const canvas = canvasEl.value
  canvas.width = window.innerWidth
  canvas.height = window.innerHeight

  await document.fonts.load(`bold 160px 'Pirata One'`).catch(() => {})

  const piecePositions = getPiecePositions()
  const targets = getTextTargets(canvas.width, canvas.height, winner.value === Cell.BLACK ? 'GG Black!' : 'GG White!')

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
      burnFrames: 0,
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

  frameCount++

  // ——— CLOSING burst animation ———
  if (isClosing.value) {
    closingFrameCount++
    const t = Math.min(closingFrameCount / CLOSE_DURATION, 1)
    // easeOutExpo: fast initial burst that slows at the end
    const eased = t === 1 ? 1 : 1 - Math.pow(2, -10 * t)

    const backdropAlpha = 0.98 * Math.max(0, 1 - eased)
    ctx.clearRect(0, 0, canvas.width, canvas.height)
    ctx.fillStyle = `rgba(45, 52, 66, ${backdropAlpha})`
    ctx.fillRect(0, 0, canvas.width, canvas.height)

    const particleAlpha = Math.max(0, 1 - eased * 1.6)
    for (const p of particles) {
      p.vy += 0.25
      p.vx *= 0.975
      p.vy *= 0.975
      p.x += p.vx
      p.y += p.vy
      if (particleAlpha > 0.01) {
        ctx.globalAlpha = particleAlpha
        ctx.beginPath()
        ctx.arc(p.x, p.y, p.radius, 0, Math.PI * 2)
        ctx.fillStyle = p.color
        ctx.fill()
        ctx.globalAlpha = 1
      }
    }

    if (t >= 1) {
      isClosing.value = false
      emit('close')
      return
    }
    animFrame = requestAnimationFrame(animate)
    return
  }

  // ——— Normal entrance animation ———
  const exploding = frameCount <= EXPLODE_DURATION

  // ✅ Calculer l'opacité du fond
  const FADE_OUT_START = 150
  const FADE_OUT_DURATION = 60
  const fadeFrame = frameCount - FADE_OUT_START
  let backdropAlpha = 0.98

  if (fadeFrame >= 0 && fadeFrame <= FADE_OUT_DURATION) {
    backdropAlpha = 0.98 * Math.max(0, 1 - (fadeFrame / FADE_OUT_DURATION))
  } else if (fadeFrame > FADE_OUT_DURATION) {
    backdropAlpha = 0
  }

  // ✅ CLEAR complètement le canvas
  ctx.clearRect(0, 0, canvas.width, canvas.height)

  // Semi-transparent fill avec opacité dynamique
  ctx.fillStyle = `rgba(45, 52, 66, ${backdropAlpha})`
  ctx.fillRect(0, 0, canvas.width, canvas.height)

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

  // Laser / eraser: burn particles near the cursor and draw the glow
  const lp = laserPos
  if (lp) {
    for (const p of particles) {
      const ldx = p.x - lp.x
      const ldy = p.y - lp.y
      const ldist = Math.sqrt(ldx * ldx + ldy * ldy) || 1
      if (ldist < LASER_RADIUS) {
        // Keep mild repulsion so particles aren't pinned instantly
        const force = LASER_FORCE * Math.pow(1 - ldist / LASER_RADIUS, 1.5)
        p.vx += (ldx / ldist) * force
        p.vy += (ldy / ldist) * force
        p.burnFrames++
      } else {
        // Slow cool-down when out of range
        p.burnFrames = Math.max(0, p.burnFrames - 0.4)
      }
    }

    // Remove particles that have reached the burn threshold
    const prevCount = particles.length
    particles = particles.filter(p => p.burnFrames < LASER_BURN_THRESHOLD)

    // Chain reaction: fewer than threshold particles left → explode all
    if (prevCount !== particles.length && !isClosing.value && particles.length < CHAIN_REACTION_THRESHOLD) {
      handleClose()
    }

    // Draw laser glow
    const gradient = ctx.createRadialGradient(lp.x, lp.y, 0, lp.x, lp.y, LASER_RADIUS)
    gradient.addColorStop(0, 'rgba(211, 80, 19, 0.22)')
    gradient.addColorStop(0.45, 'rgba(211, 80, 19, 0.08)')
    gradient.addColorStop(1, 'rgba(211, 80, 19, 0)')
    ctx.beginPath()
    ctx.arc(lp.x, lp.y, LASER_RADIUS, 0, Math.PI * 2)
    ctx.fillStyle = gradient
    ctx.fill()
    ctx.beginPath()
    ctx.arc(lp.x, lp.y, 3.5, 0, Math.PI * 2)
    ctx.fillStyle = 'rgba(211, 80, 19, 0.9)'
    ctx.fill()
  } else {
    // Cool all particles when laser is off
    for (const p of particles) {
      p.burnFrames = Math.max(0, p.burnFrames - 0.4)
    }
  }

  // Draw heat glow on particles that are accumulating burn
  for (const p of particles) {
    if (p.burnFrames > 0) {
      const heat = Math.min(p.burnFrames / LASER_BURN_THRESHOLD, 1)
      ctx.globalAlpha = heat * 0.75
      ctx.beginPath()
      ctx.arc(p.x, p.y, p.radius * (1 + heat * 1.8), 0, Math.PI * 2)
      ctx.fillStyle = `rgb(255, ${Math.round(180 * (1 - heat))}, 0)`
      ctx.fill()
      ctx.globalAlpha = 1
    }
  }

  animFrame = requestAnimationFrame(animate)
}

function handleClose() {
  // Respect prefers-reduced-motion: skip animation, close instantly
  if (window.matchMedia('(prefers-reduced-motion: reduce)').matches) {
    emit('close')
    return
  }

  if (isClosing.value) return
  isClosing.value = true

  // Fade the card out immediately
  cardVisible.value = false

  const canvas = canvasEl.value
  if (!canvas || particles.length === 0) {
    // No canvas / particles yet – derive delay from CLOSE_DURATION (assume 60 fps)
    setTimeout(() => emit('close'), Math.round((CLOSE_DURATION / 60) * 1000))
    return
  }

  const cx = canvas.width / 2
  const cy = canvas.height / 2

  // Burst all particles outward from screen center with random spread
  for (const p of particles) {
    const dx = p.x - cx
    const dy = p.y - cy
    const len = Math.sqrt(dx * dx + dy * dy) || 1
    const burstSpeed = 18 + Math.random() * 22
    p.vx = (dx / len) * burstSpeed + (Math.random() - 0.5) * 12
    p.vy = (dy / len) * burstSpeed + (Math.random() - 0.5) * 12
  }

  closingFrameCount = 0
}

onUnmounted(() => {
  cancelAnimationFrame(animFrame)
})
const cardPosition = ref({ x: 0, y: 0 })
const isDragging = ref(false)
let dragStart = { x: 0, y: 0 }
let dragOffset = { x: 0, y: 0 }

function onMouseDown(e: MouseEvent) {
  isDragging.value = true
  dragStart = { x: e.clientX, y: e.clientY }
  dragOffset = { ...cardPosition.value }
}

function onMouseMove(e: MouseEvent) {
  if (!isDragging.value) return

  const deltaX = e.clientX - dragStart.x
  const deltaY = e.clientY - dragStart.y

  cardPosition.value = {
    x: dragOffset.x + deltaX,
    y: dragOffset.y + deltaY
  }
}

function onMouseUp() {
  isDragging.value = false
}

function handleResize() {
  if (!canvasEl.value || !winner.value) return
  const canvas = canvasEl.value
  canvas.width = window.innerWidth
  canvas.height = window.innerHeight

  const screenCenterX = canvas.width / 2
  const screenCenterY = canvas.height / 2

  for (const p of particles) {
    const dx = p.targetX - screenCenterX
    const dy = p.targetY - screenCenterY
    p.vx = dx * 0.055
    p.vy = dy * 0.055
  }
}

const isClickDragging = ref(false)
let clickDragStart = { x: 0, y: 0 }

// Laser / eraser – repulsion field around the cursor
let laserPos: { x: number; y: number } | null = null
const LASER_RADIUS = 120
const LASER_FORCE = 14
const LASER_BURN_THRESHOLD = 90  // ~1.5 s of continuous contact at 60 fps
const CHAIN_REACTION_THRESHOLD = 20

function onBackdropMouseDown(event: MouseEvent) {
  if (event.target !== event.currentTarget) return

  if (event.button === 2) {
    // Right click: activate laser / eraser
    laserPos = { x: event.clientX, y: event.clientY }
  } else if (event.button === 0) {
    // Left click: drag / pull animation
    isClickDragging.value = true
    clickDragStart = { x: event.clientX, y: event.clientY }

    for (const p of particles) {
      const dx = clickDragStart.x - p.x
      const dy = clickDragStart.y - p.y
      const len = Math.sqrt(dx * dx + dy * dy) || 1
      p.vx = (dx / len) * 8
      p.vy = (dy / len) * 8
    }
  }
}

function onBackdropMouseMove(event: MouseEvent) {
  // Update laser position only while right button is held (buttons bitmask bit 2)
  if (event.buttons & 2) {
    laserPos = { x: event.clientX, y: event.clientY }
  } else if (!(event.buttons & 2)) {
    laserPos = null
  }

  if (!isClickDragging.value) return

  const currentX = event.clientX
  const currentY = event.clientY
  const deltaX = currentX - clickDragStart.x
  const deltaY = currentY - clickDragStart.y
  const distance = Math.sqrt(deltaX * deltaX + deltaY * deltaY)

  if (distance < 10) return

  for (const particle of particles) {
    const particleDx = particle.x - clickDragStart.x
    const particleDy = particle.y - clickDragStart.y
    const particleDistance = Math.sqrt(particleDx * particleDx + particleDy * particleDy)

    const pullRadius = distance * 0.5
    const pullStrength = Math.max(0, 1 - particleDistance / pullRadius)

    if (pullStrength > 0) {
      const directionX = deltaX / distance
      const directionY = deltaY / distance

      particle.vx += directionX * distance * 0.15 * pullStrength
      particle.vy += directionY * distance * 0.15 * pullStrength
    }
  }
}

function onBackdropMouseLeave() {
  laserPos = null
  isClickDragging.value = false
}

function onBackdropMouseUp(event: MouseEvent) {
  if (event.button === 2) {
    laserPos = null
  } else if (event.button === 0) {
    isClickDragging.value = false
  }
}

function onBackdropContextMenu(event: MouseEvent) {
  event.preventDefault()
}

const isShiftPressed = ref(false)

function handleKeyDown(event: KeyboardEvent) {
  if (event.key === 'Shift')
    isShiftPressed.value = true
}

function handleKeyUp(event: KeyboardEvent) {
  if (event.key === 'Shift')
    isShiftPressed.value = false
}

async function onOpen() {
  document.addEventListener('mousemove', onMouseMove)
  document.addEventListener('mouseup', onMouseUp)
  window.addEventListener('resize', handleResize)
  window.addEventListener('keydown', handleKeyDown)
  window.addEventListener('keyup', handleKeyUp)
  isShiftPressed.value = false;
  await nextTick()
  startAnimation()
  const backdrop = document.querySelector('.wm-backdrop') as HTMLElement
  backdrop?.addEventListener('mousedown', onBackdropMouseDown)
  backdrop?.addEventListener('mousemove', onBackdropMouseMove)
  backdrop?.addEventListener('mouseup', onBackdropMouseUp)
  backdrop?.addEventListener('mouseleave', onBackdropMouseLeave)
  backdrop?.addEventListener('contextmenu', onBackdropContextMenu)
}

function onLeave() {
  document.removeEventListener('mousemove', onMouseMove)
  document.removeEventListener('mouseup', onMouseUp)
  window.removeEventListener('resize', handleResize)
  window.removeEventListener('keydown', handleKeyDown)
  window.removeEventListener('keyup', handleKeyUp)
  isShiftPressed.value = false;
  isClosing.value = false
  closingFrameCount = 0
  laserPos = null
  cancelAnimationFrame(animFrame)
  particles = []
  isDragging.value = false
  cardPosition.value = { x: 0, y: 0 }
  const backdrop = document.querySelector('.wm-backdrop') as HTMLElement
  backdrop?.removeEventListener('mousedown', onBackdropMouseDown)
  backdrop?.removeEventListener('mousemove', onBackdropMouseMove)
  backdrop?.removeEventListener('mouseup', onBackdropMouseUp)
  backdrop?.removeEventListener('mouseleave', onBackdropMouseLeave)
  backdrop?.removeEventListener('contextmenu', onBackdropContextMenu)
}

watch(open, async (newVal) => {
  if (!newVal)
    return onLeave()
  onOpen()
}, { flush: 'post' }) // The handler is fired after vue has updated the component and has updated the Browser DOM.

onMounted(onOpen)
onUnmounted(onLeave)
</script>

<template>
  <teleport to="body">
    <div v-if="open" class="wm-backdrop" aria-hidden="false"
        :style="{ pointerEvents: !isShiftPressed ? 'auto' : 'none' }">
      <canvas ref="canvasEl" class="wm-canvas" aria-hidden="true"></canvas>

      <div
        class="wm-card"
        role="dialog"
        aria-modal="true"
        aria-label="Game over"
        @mousedown="onMouseDown"
        :class="{ visible: cardVisible, closing: isClosing }"
        :style="{
          transform: `translate(${cardPosition.x}px, ${cardPosition.y}px)`,
          cursor: isDragging ? 'grabbing' : 'grab',
          userSelect: isDragging ? 'none' : 'auto'
        }"
      >
        <div class="wm-title">The winner is</div>
        <div class="wm-winner" :style="{ color: winnerColor }">{{ winnerLabel }}!</div>
        <div class="wm-player" v-if="playerType">
          <span class="wm-badge">{{ playerType === 'AI' ? '🤖 AI' : '👤 Human' }}</span>
        </div>
        <button class="wm-close-btn" type="button" @click="handleClose">Close</button>
      </div>
      <div v-if="!isShiftPressed" class="shift-hint">
        Hold Shift to interact with elements
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
  cursor: crosshair;
}

.wm-canvas {
  position: absolute;
  inset: 0;
  width: 100%;
  height: 100%;
  pointer-events: none;
  opacity: 0.98;
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
  opacity: 0;
  transition: transform 0.1s ease-out;
  transition: opacity 1s linear;
  &.visible {
    opacity: 1;
  }
  &.closing {
    opacity: 0 !important;
    transition: opacity 0.45s ease-out !important;
  }
}

.wm-title {
  font-family: var(--ui-font-family);
  font-size: 1.1rem;
  color: var(--primary-color);
  opacity: 0.8;
  letter-spacing: 0.05em;
  user-select: none;
}

.wm-winner {
  user-select: none;
  font-family: var(--title-font-family);
  font-size: 3.5rem;
  font-weight: 400;
  line-height: 1;
}

.wm-player {
  margin-top: 0.1rem;
  user-select: none;
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

.shift-hint {
  position: fixed;
  top: 20px;
  left: 50%;
  transform: translateX(-50%);
  background: rgba(0, 0, 0, 0.7);
  color: rgba(255, 255, 255, 0.6);
  padding: 8px 16px;
  border-radius: 6px;
  font-size: 12px;
  pointer-events: none;
  animation: fadeInOut 3s ease-in-out;
}

@keyframes fadeInOut {
  0%, 100% { opacity: 0; }
  10%, 90% { opacity: 1; }
}
</style>
