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
import { searchOptions } from '@/helpers/helpers'

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
const searchDepth = computed(() => gameStore.gameState.searchDepth)
</script>

<template>
  <div class="layout">
  <div class="header">
    <div id="logo">
      <h1 @click="reload">Gomoku</h1>
      <p class="desc" v-if="aiGame" :title="searchFunction?.desc">
        Using <span v-if="searchFunction" @click="configOpen = true">{{ searchFunction.label }}</span>
        to depth <span @click="configOpen = true">{{ searchDepth }}</span>.
      </p>
      <span class="rules" type="button" @click="rulesOpen = true">?</span>
      <span class="config" type="button" @click="configOpen = true"><ConfigIcon size="16"/></span>
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
    right: -1rem;
    bottom: -0.2rem;
    z-index: 20;
    cursor: default;
    color: var(--white-color);

    span {
      font-size: 0.8rem;
      color: var(--accent-color);
      cursor: pointer;
    }
  }
}

.rules, .config {
  cursor: pointer;
  position: absolute;
  padding: 5px;
  font-size: 1.4rem;
  font-weight: bold;
  color: var(--text-color);
}

.rules {
  top: 0px;
  left: -20px;
}

.config {
  top: 30px;
  left: -25px;
}

h1::after {
  content: '✏️';
  position: absolute;
  font-size: 2rem;
  right: -1.5rem;
  top: -3rem;
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
