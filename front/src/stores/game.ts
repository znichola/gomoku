import { reactive } from 'vue'
import { defineStore } from 'pinia'
import type { GameState } from '../types/game'

export const useGameStore = defineStore('game', () => {
  const gameState = reactive<GameState>({ isHumanGame: false, moveHistory: [], board: null })

  function updateGameState(newgameState: GameState) {
    gameState.board = newgameState.board
    gameState.isHumanGame = newgameState.isHumanGame
    gameState.moveHistory = newgameState.moveHistory
  }

  /* >> DEBUG BACK WATCHER */
  let watcherStarted = false
  /**
   *
   * @param newgameState
   * @param resp
   * @returns true for normal behavior, false to not call updateGameState and use custom/debug watcher behavior
   */

  let serverStartAt: string | null = null
  function checkResponse(newgameState: GameState, resp: Response) {
    const startAt = resp.headers.get('expires')
    if (serverStartAt === null) {
      serverStartAt = startAt
    } else if (serverStartAt !== startAt) {
      if (!gameState.board) {
        serverStartAt = startAt
        return true
      }
      fetch('http://localhost:9012/debug-action?action=load-game-state&' + new URLSearchParams({
        isHumanGame: gameState.isHumanGame ? 'true' : 'false',
        moveHistory: gameState.moveHistory.join(','),
        board_grid: gameState.board.grid.join(','),
        board_boardDimension: gameState.board.boardDimension.toString(),
        board_blackCaptured: gameState.board.blackCaptured.toString(),
        board_whiteCaptured: gameState.board.whiteCaptured.toString(),
        board_isBlackToPlay: gameState.board.isBlackToPlay ? 'true' : 'false'
      }).toString())
        .then((response) => response.json())
        .then((data) => {
          const startAt = resp.headers.get('expires')
            if (resp.status != 200)
              console.warn('watcher: STATUS NOT 200')
          if (!startAt) return
          console.log('server restarted !')
          updateGameState(data)
          serverStartAt = startAt
        })
      return false
    }
    return true
  }

  async function watchServer() {
    try {
      const resp = await fetch('http://localhost:9012/gameState')
      if (resp.status != 200)
        throw Error('STATUS NOT 200')
      const data = await resp.json()
      checkResponse(data, resp)
    } catch {
      console.warn('server offline');
    }
  }

  let _watch_interval = 0
  function backWatcher(type: string = 'none') {
    console.log(type, watcherStarted)
    if (type === 'mounted' && !watcherStarted) {
      console.log('ok!')
      watcherStarted = true
      clearInterval(_watch_interval)
      _watch_interval = setInterval(watchServer, 1000)
    } else if (type === 'unMounted' && watcherStarted) {
      watcherStarted = false
      clearInterval(_watch_interval)
      console.info('unMounted')
    }
    if (!watcherStarted) {
      return {
        checkResponse: (_newgameState: GameState, _resp: Response) => true
      }
    }
    return {
      checkResponse
    }
  }
  /* << */

  return { gameState, updateGameState, backWatcher }
})
