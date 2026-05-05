<script setup lang="ts">
import { computed, ref } from 'vue'
import { useGameStore } from '@/stores/game'

const gameStore = useGameStore()

import controles from './Controles.vue'
import game from "./Game.vue"
import hud from './Hud.vue'
import turnlog from './TurnLog.vue'
import messages from './Messages.vue'
import RulesModal from './RulesModal.vue'
import ConfigModal from './ConfigModal.vue'
import WinnerModal from './WinnerModal.vue'
import ConfigIcon from "./icons/ConfigIcon.vue";
import { searchOptions, moveOptions } from '@/helpers/helpers'

const reload = () => window.location.reload()

const rulesOpen = ref(sessionStorage.getItem('gomoku_rules') !== 'true')
const configOpen = ref(false)

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

function closeRules() {
  rulesOpen.value = false;
  // sessionStorage: Open at each new browser session, new session happened at browser restart
  sessionStorage.setItem('gomoku_rules', 'true');
}

const aiGame = computed(() => gameStore.gameState.isAIGame)
const searchFunction = computed(() => searchOptions.find((a) => a.value === gameStore.gameState.searchFunction) || null)
const moveFunction = computed(() => moveOptions.find((a) => a.value === gameStore.gameState.moveFunction) || null)
const searchDepth = computed(() => gameStore.gameState.searchDepth)
</script>

<template>
  <div class="layout">
  <div class="header">
    <div class="btns">
      <button class="helper-btn" type="button" @click="rulesOpen = true">?</button>
      <button class="helper-btn" type="button" @click="configOpen = true"><ConfigIcon size="16"/></button>
      <router-link class="helper-btn" to="/pp">P</router-link>
    </div>
    <div id="logo">
      <h1 @click="reload">Gomoku</h1>
      <p class="desc" v-if="aiGame" :title="searchFunction?.desc">
        Using <span v-if="searchFunction" @click="configOpen = true">{{ searchFunction.label }}</span>
        with <span v-if="moveFunction" @click="configOpen = true">{{ moveFunction.label }}</span>
        to depth <span @click="configOpen = true">{{ searchDepth }}</span>.
      </p>

    </div>

    <controles />
    <hud />
    <messages />
  </div>
  <game class="game" />
  <turnlog class="turnlog" />
</div>
<RulesModal :open="rulesOpen" @close="closeRules" />
<ConfigModal :open="configOpen" @close="configOpen = false" />
<WinnerModal @close="reset()" />
</template>

<style scoped lang="less">
h1 {
  cursor: pointer;
}

.layout {
  max-width: var(--content-width);
  height: 100vh;
}

.header {
  height: min-content;
  display: flex;
  align-items: center;
  justify-content: center;
  flex-wrap: wrap;
  padding-top: 0.8rem;
}

#logo {
  position: relative;
  p.desc {
    font: var(--ui-font);
    font-size: 0.8rem;
    position: absolute;
    left: 2.4rem;
    bottom: -0.4rem;
    z-index: 20;
    cursor: default;
    color: var(--white-color);
    width: calc(100% - 36px);
    white-space: nowrap;

    span {
      font-size: 0.8rem;
      color: var(--accent-color);
      cursor: pointer;
    }
  }
}

.helper-btn {
  cursor: pointer;
  font: var(--ui-font);
  font-size: 1.6rem;
  font-weight: bold;
  color: var(--text-color);
  padding: 0.05rem;
  z-index: 100;
  text-decoration: none;
}

.helper-btn:hover {
  color: var(--accent-color);
}

.btns {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding-right: 0.2rem;
  button, a {
    background-color: initial;
  }
}

h1::after {
  content: '✏️';
  position: absolute;
  font-size: 2rem;
  right: -1.5rem;
  top: -3.5rem;
  transition: all 0.5s ease-in-out;
}

h1::before {
  content: '⌨️';
  position: absolute;
  font-size: 1.8rem;
  left: 0rem;
  bottom: 6rem;
  transition: all 0.5s ease-in-out;
}

.editMode h1::after {
  top: 0;
}

.keyMode h1::before {
  bottom: -1rem;
}
</style>
