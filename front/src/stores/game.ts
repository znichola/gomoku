import { reactive } from 'vue'
import { defineStore } from 'pinia'
import type { GameState } from '../types/game'

export const useGameStore = defineStore('game', () => {
  const gameState = reactive<GameState>({ isHumanGame: false, moveHistory: [], board: null })
  const watcherState = reactive({
    enabled: false,
    preview: false,
    edition: false
  })

  function updateGameState(newgameState: GameState) {
    gameState.board = newgameState.board
    gameState.isHumanGame = newgameState.isHumanGame
    gameState.moveHistory = newgameState.moveHistory
  }

  /* >> DEBUG BACK WATCHER */
  /**
   *
   * @param newgameState
   * @param resp
   * @returns true for normal behavior, false to not call updateGameState and use custom/debug watcher behavior
   */

  function makeGameStateQuery(): string {
    if (!gameState.board)
      return ''
    return new URLSearchParams({
      isHumanGame: gameState.isHumanGame ? 'true' : 'false',
      moveHistory: gameState.moveHistory.join(','),
      board_grid: gameState.board.grid.join(','),
      board_boardDimension: gameState.board.boardDimension.toString(),
      board_blackCaptured: gameState.board.blackCaptured.toString(),
      board_whiteCaptured: gameState.board.whiteCaptured.toString(),
      board_isBlackToPlay: gameState.board.isBlackToPlay ? 'true' : 'false'
    }).toString()
  }

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
      const query = localStorage.getItem('gomoku-watcher-T0') ?? makeGameStateQuery()
      fetch('http://localhost:9012/debug-action?action=load-game-state&' + query)
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

  function setT0() {
    if (!gameState.board)
      return
    localStorage.setItem('gomoku-watcher-T0', makeGameStateQuery())
    localStorage.setItem('gomoku-watcher-preview', gameState.board.grid.join(','))
    console.info('Set T0.')
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
    if (type === 'mounted' && !watcherState.enabled) {
      if (localStorage.getItem('gomoku-watcher-enabled') === 'true') {
        return backWatcher('start')
      }
    } else if (type === 'start' && !watcherState.enabled) {
      console.info('Watcher started!')
      localStorage.setItem('gomoku-watcher-enabled', 'true')
      watcherState.enabled = true
      clearInterval(_watch_interval)
      _watch_interval = setInterval(watchServer, 1000)
    } else if (type === 'unMounted' && watcherState.enabled) {
      return backWatcher('stop')
    } else if (type === 'stop' && watcherState.enabled) {
      localStorage.setItem('gomoku-watcher-enabled', 'false')
      watcherState.enabled = false
      clearInterval(_watch_interval)
    }
    if (!watcherState.enabled) {
      return {
        checkResponse: (_newgameState: GameState, _resp: Response) => true,
        setT0
      }
    }
    return {
      checkResponse,
      setT0
    }
  }
  /* << */

  return { gameState, updateGameState, backWatcher, watcherState }
})
