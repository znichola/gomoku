<script setup lang="ts">
import { useGameStore } from '@/stores/game'
import { onMounted, onUnmounted, computed } from 'vue'

const gameStore = useGameStore()

const aiGame = computed(() => gameStore.gameState.isAIGame)

onMounted(() => window.addEventListener('click', click, true))
onUnmounted(() => window.removeEventListener('click', click, true))

async function click(event: MouseEvent) {
  if (!event.target || !(gameStore.watcherState.edition || (gameStore.watcherState.keymode && event.altKey)))
    return
  const target = (event.target as HTMLElement)
  let element: HTMLElement | null = null
  let action: [string, string] | null = null
  if ((element = target.closest('div.circle'))) {
    if (element.getAttribute('data-type') === 'black' || element.getAttribute('data-type') === 'white') {
      action = ['erase', element.id]
    }
  } else if ((element = target.closest('div.capture-card'))) {
    action = ['minus', element.classList.contains('black') ? '1' : '2']
  }
  if (!action)
    return
  event.stopPropagation()
  event.preventDefault()
  try {
    const resp = await fetch(`http://${window.location.hostname}:9012/debug-action?action=${action[0]}&id=${action[1]}`, {
      method: 'GET',
    })
    const data = await resp.json()
    gameStore.updateGameState(data);
    } catch (err) {
    console.warn(err)
  }
}

async function toggleAI(color: 1 | 2) {
  try {
    const v = gameStore.gameState.isAIGame == color ? 0 : color
    const resp = await fetch(`http://${window.location.hostname}:9012/set-config?isAIGame=${v}`, {
      method: 'GET',
    })
    const data = await resp.json()
    gameStore.updateGameState(data);
    } catch (err) {
    console.warn(err)
  }
}

async function reset() {
  try {
    const resp = await fetch(`http://${window.location.hostname}:9012/reset`, {
      method: 'GET',
    })
    const data = await resp.json()
    gameStore.updateGameState(data);
    } catch (err) {
    console.warn(err)
  }
}

async function debug(action: string) {
  try {
    const resp = await fetch(`http://${window.location.hostname}:9012/debug-action?action=${action}`, {
      method: 'GET',
    })
    const data = await resp.json()
    console.log(data)
    gameStore.updateGameState(data);
    } catch (err) {
    console.warn(err)
  }
}

function watcher(action: string) {
  if (action === 'start') {
    gameStore.backWatcher('start')
  } else if (action === 'stop') {
    gameStore.backWatcher('stop')
  } else if (action === 'set-t0') {
    gameStore.backWatcher().setT0()
  } else if (action === 'apply-t0') {
    gameStore.backWatcher().applyT0()
  } else if (action === 'reset-t0') {
    gameStore.backWatcher().resetT0()
  } else if (action === 'toggle-edit') {
    gameStore.watcherState.edition = !gameStore.watcherState.edition
    console.log('Switch edit mode', gameStore.watcherState.edition)
  } else if (action === 'toggle-keymode') {
    gameStore.watcherState.keymode = !gameStore.watcherState.keymode
    console.log('Switch cheating mode', gameStore.watcherState.keymode)
  } else {
    console.warn('action not found')
  }
}

function preview(state: boolean) {
  gameStore.watcherState.preview = state
}

</script>

<template>
<div class="controles">
  <button class="reset-btn" @click="reset">Restart</button>
  <div class="menu">
    <span :class="{reverse: aiGame !== 0}">IA</span>
    <ul>
      <li>
        <button class="ai-play" :class="{reverse: aiGame === 1}" @click="toggleAI(1)"
          >AI {{(aiGame === 1) ? 'dis' : 'on'}} Black</button>
      </li>
      <li>
        <button class="ai-play" :class="{reverse: aiGame === 2}" @click="toggleAI(2)"
          >AI {{(aiGame === 2) ? 'dis' : 'on'}} White</button></li>
      <li>
        <div>
          <label for="aispeed">Speed ({{ +gameStore.watcherState.speed }}s):</label>
          <input id="aispeed" type="range" :value="gameStore.watcherState.speed"
            @input="event => gameStore.watcherState.speed = (event.target as any).value || 0" min="0" max="10">
        </div>
      </li>
    </ul>
  </div>
  <div class="menu">
    <span>Debug</span>
    <ul>
      <li>
        <button class="debug-btn" @click="debug('make-double-tree')"
            title="Créer un double three selon la position de la dernière pièce, dans une rotation aléatoire."
          >Double Tree Maker</button>
      </li>
      <li>
        <button class="debug-btn" @click="debug('set-reset')"
            title="Définit le reset sur l'état actuel de la grille ."
          >Set Restart</button>
        </li>
      <li>
        <button class="debug-btn" @click="watcher('start')" v-if="!gameStore.watcherState.enabled"
            title="Démarre le Watcher. Si le serveur redémarre le GameState actuel ou T0 sera envoyé."
          >Start Watcher</button>
        <button class="debug-btn reverse" @click="watcher('stop')" v-else
            title="Arrête le Watcher. Si le serveur redémarre l'état du jeu sera reset."
          >Stop Watcher</button>
      </li>
      <li>
        <button class="debug-btn" @click="watcher('set-t0')" @mouseover="preview(true)" @mouseleave="preview(false)"
            title="Utilise le GameState actuel comme point de rollback pour chaque redémarrage du serveur."
          >Watcher Set T0</button>
      </li>
      <li>
        <button class="debug-btn" @click="watcher('apply-t0')" @mouseover="preview(true)" @mouseleave="preview(false)"
            title="Applique immédiatement T0."
          >Apply T0</button>
      </li>
      <li>
        <button class="debug-btn" @click="watcher('reset-t0')"
            title="Supprime T0, le dernier état de GameState dans le navigateur sera utilisé comme point de rollback."
          >Reset T0</button>
      </li>
      <li>
        <button class="debug-btn" @click="watcher('toggle-edit')" :class="{ reverse: gameStore.watcherState.edition }"
            title="Active/Désactive le mode édition au clique. Permet de supprimer une pièce ou de diminuer le score de capture"
          >Toggle Edit</button>
      </li>
      <li>
        <button class="debug-btn" @click="watcher('toggle-keymode')" :class="{ reverse: gameStore.watcherState.keymode }"
            title="Active/Désactive les modifieurs clavier. SHIFT + CLICK => BLACK - CTRL + CLICK => WHITE - ALT + CLICK => EMPTY"
          >Key Modifiers</button>
      </li>
    </ul>
  </div>
</div>
</template>

<style scoped lang="less">
input[type=range]::after {
  content: attr(value);
}

.controles {
  padding: 0.3em;
  display: flex;
  align-items: center;
  gap: 1rem;
  margin-left: 2rem;
}

div.menu {
  position: relative;
  ul {
    display: none;
    position: absolute;
    padding-top: 0.5rem;
    li {
      width: max-content;
      padding-bottom: 0.5rem;
    }
  }
  &:hover ul, & ul:hover {
    display: block;
    z-index: 400;
  }
}

div.menu li > div {
  cursor: pointer;
  display: flex;
  padding: 0.2rem 0.3rem;;
  border: solid 0.165rem var(--accent-color);
  border-radius: 1.2rem;
  background-color: var(--primary-color);
  color: var(--bg-color);
  font: var(--ui-font);
  transition: all 0.3s ease;
  align-items: center;
}

button, div.menu span {
  cursor: pointer;
  display: block;
  padding: 0.2rem 0.3rem;;
  border: solid 0.165rem var(--accent-color);
  border-radius: 1.2rem;
  background-color: var(--primary-color);
  color: var(--bg-color);
  font: var(--ui-font);
  transition: all 0.3s ease;

  &:hover, &.reverse {
    background-color: var(--accent-color);
    color: var(--primary-color);
  }

  &.reverse:hover {
    background-color: var(--primary-color);
    color: var(--bg-color);
  }

  &:active {
    transform: scale(0.95);
  }
}

</style>
