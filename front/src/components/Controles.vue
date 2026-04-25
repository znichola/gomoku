<script setup lang="ts">
import { useGameStore } from '@/stores/game'
import { Cell } from '@/types/game'
import { onMounted, onUnmounted } from 'vue'

const gameStore = useGameStore()

onMounted(() => window.addEventListener('click', click, true))
onUnmounted(() => window.removeEventListener('click', click, true))

async function click(event: MouseEvent) {
  if (!event.target || !(gameStore.watcherState.edition || (gameStore.watcherState.keymode && event.altKey))) {
    if (event.ctrlKey) {
      const target = (event.target as HTMLElement)
      let element: HTMLElement | null = null
      if ((element = target.closest('div.capture-card'))) {
        const cell = element.classList.contains('black') ? Cell.BLACK : Cell.WHITE
        gameStore.watcherState.human = gameStore.watcherState.human === cell ? 0 : cell
      }
    }
    return
  }
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
    localStorage.setItem('gomoku_editmode', gameStore.watcherState.edition.toString())
    console.log('Switch edit mode', gameStore.watcherState.edition)
  } else if (action === 'toggle-keymode') {
    gameStore.watcherState.keymode = !gameStore.watcherState.keymode
    localStorage.setItem('gomoku_keymode', gameStore.watcherState.keymode.toString())
    console.log('Switch key mode', gameStore.watcherState.keymode)
  } else {
    console.warn('action not found')
  }
}

onMounted(() => {
  if (localStorage.getItem('gomoku_editmode') === 'true') {
    gameStore.watcherState.edition = true;
  }
  if (localStorage.getItem('gomoku_keymode') === 'true') {
    gameStore.watcherState.keymode = true;
  }
})

function preview(state: boolean) {
  gameStore.watcherState.preview = state
}

</script>

<template>
<div class="controles">
  <button class="reset-btn" @click="reset">Restart</button>
  <div class="menu">
    <span id="debug-span">Debug</span>
    <ul>
      <li>
        <button class="debug-btn" @click="watcher('toggle-keymode')" :class="{ reverse: gameStore.watcherState.keymode }"
            title="Active/Désactive les modifieurs clavier. SHIFT + CLICK => BLACK - CTRL + CLICK => WHITE - ALT + CLICK => EMPTY"
          >Key Modifiers</button>
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
        <button class="debug-btn" @click="watcher('apply-t0')" @mouseover="preview(true)" @mouseleave="preview(false)"
            title="Applique immédiatement T0."
          >Apply T0 to board</button>
      </li>
      <li>
        <button class="debug-btn" @click="debug('set-reset')"
            title="Définit le reset sur l'état actuel de la grille ."
          >Set Restart</button>
      </li>
      <li>
        <button class="debug-btn" @click="watcher('set-t0')" @mouseover="preview(true)" @mouseleave="preview(false)"
            title="Utilise le GameState actuel comme point de rollback pour chaque redémarrage du serveur."
          >Set T0</button>
      </li>
      <li>
        <button class="debug-btn" @click="watcher('reset-t0')"
            title="Supprime T0, le dernier état de GameState dans le navigateur sera utilisé comme point de rollback."
          >Erase T0</button>
      </li>
      <li>
        <button class="debug-btn" @click="watcher('toggle-edit')" :class="{ reverse: gameStore.watcherState.edition }"
            title="Active/Désactive le mode édition au clique. Permet de supprimer une pièce ou de diminuer le score de capture"
          >Toggle Edit</button>
      </li>
      <li>
        <button class="debug-btn" @click="debug('make-double-tree')"
            title="Créer un double three selon la position de la dernière pièce, dans une rotation aléatoire."
          >Double Tree Maker</button>
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

#debug-span {
  position: relative;
}
#debug-span::after {
  content: '📤';
  position: absolute;
  font-size: 1rem;
  right: -1rem;
  top: -3.5rem;
  transition: all 0.5s ease-in-out;
}

.watcherMode #debug-span::after {
  top: 1.4rem;
}
</style>
