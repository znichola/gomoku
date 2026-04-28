<script setup lang="ts">
import { ref, watch, onUnmounted } from 'vue'
import { useGameStore } from '@/stores/game'

defineProps<{ iso3D: boolean }>()

const gameStore = useGameStore()

// ── Crowd data (static, deterministic – no Math.random() in template) ──────

type Mood = 'idle' | 'cheer' | 'wild' | 'boo'

interface Person {
  id: number
  emoji: string
  delay: number // ms, for staggered animation
}

interface CrowdRow {
  id: number
  persons: Person[]
  offsetPx: number // horizontal offset for alternating stagger
  scale: number    // relative font-size (back rows are smaller)
}

const EMOJIS = ['🧑', '👩', '👨', '🧒', '👧', '👦', '🧓', '👴', '👵']

const CROWD_ROWS: CrowdRow[] = (() => {
  const defs = [
    { count: 22, scale: 0.60, offsetPx: 0  }, // row 0 – back, most distant
    { count: 18, scale: 0.70, offsetPx: 18 },
    { count: 14, scale: 0.82, offsetPx: 0  },
    { count: 10, scale: 1.00, offsetPx: 18 }, // row 3 – front, closest
  ]
  return defs.map((def, r) => ({
    id: r,
    scale: def.scale,
    offsetPx: def.offsetPx,
    persons: Array.from({ length: def.count }, (_, i) => {
      const uid = r * 30 + i
      return {
        id: uid,
        emoji: EMOJIS[uid % EMOJIS.length]!,
        delay: (uid * 137) % 700, // deterministic pseudo-random delay
      }
    }),
  }))
})()

// ── Crowd mood & reaction banner ────────────────────────────────────────────

const crowdMood = ref<Mood>('idle')
const reactionText = ref('')
const showReaction = ref(false)

let reactionTimer: ReturnType<typeof setTimeout> | null = null

function triggerReaction(mood: Mood, text: string, duration = 2400) {
  crowdMood.value = mood
  reactionText.value = text
  showReaction.value = true
  if (reactionTimer) clearTimeout(reactionTimer)
  reactionTimer = setTimeout(() => {
    crowdMood.value = 'idle'
    showReaction.value = false
  }, duration)
}

// ── Game-event watchers ─────────────────────────────────────────────────────

let prevMoveCount = 0
let prevTotalCaptured = 0
let quietStreak = 0

const CHEER_MSGS = ['👏 Beau coup !', '⚡ Excellent !', '🤔 Stratégique…', '💡 Intéressant !']
const BOO_MSGS   = ['😤 On veut du spectacle !', '😴 Zzzz…', '👎 Réveillez-vous !']
const CAP_MSGS   = ['🎯 PRISE !', '🔥 CAPTURE !', '💥 CAPTURÉ !']

watch(
  () => gameStore.gameState.board?.winner,
  (winner) => {
    if (winner) {
      triggerReaction('wild', '🎉 VICTOIRE ! 🎉', 10000)
      launchConfetti()
    }
  },
)

watch(
  () => gameStore.gameState.moveHistory.length,
  (newLen) => {
    if (newLen <= prevMoveCount) return
    if (gameStore.gameState.board?.winner) return

    const bc = gameStore.gameState.board?.blackCaptured ?? 0
    const wc = gameStore.gameState.board?.whiteCaptured ?? 0
    const totalCaptured = bc + wc

    if (totalCaptured > prevTotalCaptured) {
      const diff = totalCaptured - prevTotalCaptured
      const label = CAP_MSGS[newLen % CAP_MSGS.length]! + (diff > 1 ? ` (×${diff})` : '')
      triggerReaction('cheer', label, 2500)
      quietStreak = 0
    } else {
      quietStreak++
      if (quietStreak >= 6 && quietStreak % 6 === 0) {
        const idx = (quietStreak / 6 - 1) % BOO_MSGS.length
        triggerReaction('boo', BOO_MSGS[Math.floor(idx)]!, 2500)
      } else {
        triggerReaction('cheer', CHEER_MSGS[newLen % CHEER_MSGS.length]!, 1800)
      }
    }

    prevMoveCount = newLen
    prevTotalCaptured = totalCaptured
  },
)

// ── Confetti ─────────────────────────────────────────────────────────────────

interface ConfettiPiece {
  id: number
  x: number        // % from left
  w: number        // px width
  h: number        // px height
  color: string
  delay: number    // seconds
  duration: number // seconds
  tilt: number     // initial rotation deg
  isCircle: boolean
}

const CONFETTI_COLORS = [
  '#ff6b6b', '#ffd93d', '#6bcb77', '#4d96ff',
  '#f7b2bd', '#c77dff', '#ffb347', '#00d4ff',
]

const confettiPieces = ref<ConfettiPiece[]>([])
const confettiActive = ref(false)
let confettiTimer: ReturnType<typeof setTimeout> | null = null

function launchConfetti() {
  const pieces: ConfettiPiece[] = []
  for (let i = 0; i < 100; i++) {
    const w = 6 + (i * 7) % 9
    const isCircle = i % 4 === 0
    pieces.push({
      id: i,
      x: i,                           // 0–99 %
      w,
      h: isCircle ? w : Math.round(w * 0.6),
      color: CONFETTI_COLORS[i % CONFETTI_COLORS.length]!,
      delay: (i * 0.041) % 4,
      duration: 3 + (i * 0.071) % 2,
      tilt: (i * 47) % 360,
      isCircle,
    })
  }
  confettiPieces.value = pieces
  confettiActive.value = true
  if (confettiTimer) clearTimeout(confettiTimer)
  confettiTimer = setTimeout(() => {
    confettiActive.value = false
    confettiPieces.value = []
  }, 11000)
}

onUnmounted(() => {
  if (reactionTimer) clearTimeout(reactionTimer)
  if (confettiTimer) clearTimeout(confettiTimer)
})
</script>

<template>
  <Teleport to="body">
    <div class="arena-overlay" :class="{ active: iso3D }" aria-hidden="true">

      <!-- Stadium-seat gradient backdrop -->
      <div class="arena-bg" />

      <!-- Crowd rows -->
      <div class="crowd-container" :class="`mood-${crowdMood}`">
        <div
          v-for="row in CROWD_ROWS"
          :key="row.id"
          class="crowd-row"
          :style="`margin-left: ${row.offsetPx}px; --row-scale: ${row.scale}`"
        >
          <span
            v-for="p in row.persons"
            :key="p.id"
            class="person"
            :style="`--delay: ${p.delay}ms`"
          >{{ p.emoji }}</span>
        </div>
      </div>

      <!-- Reaction banner -->
      <Transition name="pop">
        <div
          v-if="showReaction"
          class="reaction-banner"
          :class="`mood-${crowdMood}`"
        >
          {{ reactionText }}
        </div>
      </Transition>

      <!-- Confetti -->
      <template v-if="confettiActive">
        <div
          v-for="p in confettiPieces"
          :key="p.id"
          class="confetti-piece"
          :class="{ circle: p.isCircle }"
          :style="`
            left: ${p.x}%;
            width: ${p.w}px;
            height: ${p.h}px;
            background: ${p.color};
            animation-delay: ${p.delay}s;
            animation-duration: ${p.duration}s;
            --tilt: ${p.tilt}deg;
          `"
        />
      </template>

    </div>
  </Teleport>
</template>

<style scoped lang="less">
// ── Overlay container ────────────────────────────────────────────────────────
.arena-overlay {
  position: fixed;
  inset: 0;
  pointer-events: none;
  z-index: 2;
  overflow: hidden;
  opacity: 0;
  transition: opacity 0.6s ease;

  &.active {
    opacity: 1;
  }
}

// ── Stadium background ────────────────────────────────────────────────────────
.arena-bg {
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  height: 48%;
  background:
    linear-gradient(
      to bottom,
      rgba(18, 22, 32, 0.97) 0%,
      rgba(18, 22, 32, 0.85) 55%,
      rgba(18, 22, 32, 0.4)  80%,
      transparent            100%
    );
}

// ── Crowd ────────────────────────────────────────────────────────────────────
.crowd-container {
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 0.4rem 0 0;
  gap: 0.1rem;
}

.crowd-row {
  --row-scale: 1;
  display: flex;
  gap: 0.1rem;
}

.person {
  font-size: calc(1.4rem * var(--row-scale));
  display: inline-block;
  line-height: 1;
  // default gentle sway
  animation: crowd-idle 3.2s var(--delay, 0ms) ease-in-out infinite;
}

// ── Per-mood crowd animations ────────────────────────────────────────────────
.mood-cheer .person {
  animation: crowd-cheer 0.55s var(--delay, 0ms) ease-out 4;
}

.mood-wild .person {
  animation: crowd-wild 0.38s var(--delay, 0ms) ease-in-out infinite;
}

.mood-boo .person {
  animation: crowd-boo 1.1s var(--delay, 0ms) ease-in-out 3;
}

@keyframes crowd-idle {
  0%,  100% { transform: translateY(0)    rotate(-1.5deg); }
  50%       { transform: translateY(-2px) rotate(1.5deg);  }
}

@keyframes crowd-cheer {
  0%   { transform: translateY(0)     scale(1)    rotate(0deg);   }
  30%  { transform: translateY(-9px)  scale(1.22) rotate(-6deg);  }
  60%  { transform: translateY(-5px)  scale(1.12) rotate(5deg);   }
  100% { transform: translateY(0)     scale(1)    rotate(0deg);   }
}

@keyframes crowd-wild {
  0%   { transform: translateY(0)     scale(1)    rotate(0deg);  }
  20%  { transform: translateY(-11px) scale(1.28) rotate(-8deg); }
  50%  { transform: translateY(0)     scale(1)    rotate(0deg);  }
  70%  { transform: translateY(-7px)  scale(1.18) rotate(8deg);  }
  100% { transform: translateY(0)     scale(1)    rotate(0deg);  }
}

@keyframes crowd-boo {
  0%   { transform: rotate(0deg)    translateX(0);   }
  15%  { transform: rotate(-4deg)   translateX(-3px); }
  30%  { transform: rotate(4deg)    translateX(3px);  }
  50%  { transform: rotate(-2.5deg) translateX(-1px); }
  70%  { transform: rotate(2.5deg)  translateX(1px);  }
  100% { transform: rotate(0deg)    translateX(0);   }
}

// ── Reaction banner ───────────────────────────────────────────────────────────
.reaction-banner {
  position: absolute;
  top: 43%;
  left: 50%;
  transform: translateX(-50%);
  background: rgba(10, 14, 24, 0.75);
  color: #fff;
  padding: 0.45rem 1.3rem;
  border-radius: 2rem;
  font-size: 1.15rem;
  font-weight: bold;
  font-family: var(--title-font-family);
  white-space: nowrap;
  border: 2px solid rgba(255, 255, 255, 0.25);
  backdrop-filter: blur(6px);

  &.mood-wild {
    background: rgba(180, 60, 10, 0.88);
    border-color: #ffd93d;
    font-size: 1.5rem;
    box-shadow: 0 0 24px rgba(211, 80, 19, 0.6);
  }

  &.mood-boo {
    background: rgba(90, 30, 30, 0.88);
    border-color: rgba(255, 90, 90, 0.5);
  }

  &.mood-cheer {
    background: rgba(15, 70, 40, 0.88);
    border-color: rgba(107, 203, 119, 0.5);
  }
}

// ── Transition: reaction banner pop ─────────────────────────────────────────
.pop-enter-active {
  animation: pop-in 0.28s ease-out;
}
.pop-leave-active {
  animation: pop-out 0.25s ease-in forwards;
}

@keyframes pop-in {
  from { transform: translateX(-50%) scale(0.4); opacity: 0; }
  to   { transform: translateX(-50%) scale(1);   opacity: 1; }
}

@keyframes pop-out {
  from { transform: translateX(-50%) scale(1);   opacity: 1; }
  to   { transform: translateX(-50%) scale(0.4); opacity: 0; }
}

// ── Confetti ──────────────────────────────────────────────────────────────────
.confetti-piece {
  position: fixed;
  top: -12px;
  border-radius: 2px;
  animation: confetti-fall linear forwards;
  transform: rotate(var(--tilt, 0deg));

  &.circle {
    border-radius: 50%;
  }
}

@keyframes confetti-fall {
  0% {
    transform: translateY(0)       rotate(var(--tilt, 0deg));
    opacity: 1;
  }
  85% {
    opacity: 0.9;
  }
  100% {
    transform: translateY(110vh)   rotate(calc(var(--tilt, 0deg) + 720deg));
    opacity: 0;
  }
}
</style>
