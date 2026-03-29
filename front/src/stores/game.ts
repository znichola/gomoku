import { reactive, ref, type Ref } from 'vue'
import { defineStore } from 'pinia'
import type { Cell, GameState } from '../types/game'

export const useGameStore = defineStore('game', () => {
  const gameState = reactive<GameState>({ isHumanGame: false, moveHistory: [], board: null })
  const watcherState = reactive({
    enabled: false,
    preview_state: [] as Cell[],
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
      applyT0(true)
      return false
    }
    return true
  }

  function applyT0(updateStartAt: boolean = false) {
    let startAt: string | null = null
    const query = localStorage.getItem('gomoku-watcher-T0') ?? makeGameStateQuery()
    if (!query || query.length <= 0)
      return console.debug('No T0.')
    fetch('http://localhost:9012/debug-action?action=load-game-state&' + query)
      .then((response) => {
        if (response.status != 200) {
          console.warn('watcher: STATUS NOT 200')
        } else {
          startAt = response.headers.get('expires')
        }
        return response
      })
      .then((response) => response.json())
      .then((data) => {
        if (!startAt) return
        updateGameState(data)
        if (updateStartAt) {
          console.log('server restarted !')
          serverStartAt = startAt
        }
      })
  }

  function setT0() {
    if (!gameState.board)
      return
    localStorage.setItem('gomoku-watcher-T0', makeGameStateQuery())
    watcherState.preview_state = [...gameState.board.grid]
    localStorage.setItem('gomoku-watcher-preview', gameState.board.grid.join(','))
    console.info('Set T0.')
  }

  function resetT0() {
    localStorage.removeItem('gomoku-watcher-T0')
    watcherState.preview_state = []
    localStorage.removeItem('gomoku-watcher-preview')
    console.info('Reset T0.')
  }

  async function watchServer() {
    try {
      const resp = await fetch('http://localhost:9012/gameState?silent')
      if (resp.status != 200)
        throw Error('STATUS NOT 200')
      const data = await resp.json()
      checkResponse(data, resp)
    } catch {
      console.warn('server offline');
    }
  }

  const highlightCircle: Ref<number | null> = ref(null)
  function setHighlightCircle(cid: number, state: boolean) {
    if (!state) {
      highlightCircle.value = null
    } else {
      highlightCircle.value = cid
    }
  }

  let _watch_interval = 0
  function backWatcher(type: string = 'none') {
    if (type === 'mounted' && !watcherState.enabled) {
      const previewStr = localStorage.getItem('gomoku-watcher-preview')
      if (previewStr) {
        watcherState.preview_state = previewStr?.split(',').map((a)=>+a) as Cell[]
      }
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
      watcherState.enabled = false
      clearInterval(_watch_interval)
    } else if (type === 'stop' && watcherState.enabled) {
      localStorage.setItem('gomoku-watcher-enabled', 'false')
      return backWatcher('unMounted')
    }
    if (!watcherState.enabled) {
      return {
        checkResponse: (_newgameState: GameState, _resp: Response) => true,
        setT0, applyT0, resetT0
      }
    }
    return {
      checkResponse,
      setT0, applyT0, resetT0
    }
  }
  /* << */

  return {
    gameState, updateGameState, backWatcher, watcherState,
    highlight: {
      get: () => highlightCircle.value,
      set: setHighlightCircle
    }
  }
})
