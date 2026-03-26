import { ref, computed, reactive } from 'vue'
import { defineStore } from 'pinia'
import type { GameState } from '../types/game'

export const useGameStore = defineStore('game', () => {
  const gameState = reactive<GameState>({ isHumanGame: false, movesPlayed: [], board: null })

  function updateGameState(newgameState: GameState) {
    gameState.board = newgameState.board
    gameState.isHumanGame = newgameState.isHumanGame
    gameState.movesPlayed = newgameState.movesPlayed
  }

  return { gameState, updateGameState }
})
