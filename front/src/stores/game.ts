import { reactive, ref, type Ref } from 'vue'
import { defineStore } from 'pinia'
import { Cell, type GameState, type OverlayLayer, type OverlayMessage } from '../types/game'

// Conversion HSV → RGB
function hsvToRgb(h: number, s: number, v: number) {
  let r = 0, g = 0, b = 0;
  const i = Math.floor(h * 6);
  const f = h * 6 - i;
  const p = v * (1 - s);
  const q = v * (1 - f * s);
  const t = v * (1 - (1 - f) * s);

  switch (i % 6) {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    case 5: r = v; g = p; b = q; break;
  }

  return [
    Math.round(r * 255),
    Math.round(g * 255),
    Math.round(b * 255)
  ];
}

export const useGameStore = defineStore('game', () => {
  const gameState = reactive<GameState>({
    isAIGame: 0,
    searchFunction: 'ALPHABETA_NEGAMAX_TT',
    moveFunction: 'CANDIDATE_MOVES_2',
    moveSuggestion: 'off',
    searchDepth: 0,
    moveHistory: [],
    board: null,
    messages: []
  })
  const watcherState = reactive({
    enabled: false,
    preview_state: [] as Cell[],
    preview: false,
    edition: false,
    keymode: false,
    speed: 1,
    human: Cell.EMPTY
  })

  // Prevent server spamming
  const fetchIsAvailable: Ref<boolean> = ref(true)

  const overlayDisabled = reactive({})
  const overlayLayers = ref<OverlayLayer[]>([])
  const overlayMessages = ref<OverlayMessage[]>([])

  function tryParseOverlayMessage(raw: string) {
    const obj = (() => {
      try {
        return JSON.parse(raw)
      } catch {
        return null;
      }
    })()
    if (!obj) return null;
    if (
      obj &&
      typeof obj === 'object' &&
      typeof obj.id === 'number' &&
      typeof obj.msg === 'string'
    ) {
      if ('layer' in obj) {
        const arr = obj.layer.split(',')
        const name = arr[0]
        const element = overlayLayers.value?.find((a) => a.name === name)
        if (!element) {
          const arg2 = arr[1] || arr[0]
          const color = (() => {
            if (CSS.supports('color', arg2)) {
              return arg2
            } else if (/--\w*/.test(arg2) && window.getComputedStyle(document.body).getPropertyValue(arg2)) {
              return `var(${arg2})`
            } else {
              const i = overlayLayers.value.length
              const t = (i * 0.618033988749895) % 1.0
              const v = Math.sqrt(1.0 - ((i * 0.618033988749895) % 0.5))
              const [r, g, b] = hsvToRgb(t, 0.5, v)
              return `rgb(${r}, ${g}, ${b})`
            }
          })()
          const newElement = { name, color }
          overlayLayers.value.push(newElement)
          obj.group = newElement
        } else {
          obj.group = element
        }
      }
      return obj as OverlayMessage
    }
  }

  function processMessages(messages: string[]) {
    const remaining: string[] = []

    overlayLayers.value = []
    overlayMessages.value = []
    for (const m of messages) {
      const parsed = tryParseOverlayMessage(m)
      if (parsed) overlayMessages.value.push(parsed)
      else remaining.push(m)
    }

    return remaining
  }

  function updateGameState(newgameState: GameState) {
    gameState.board = newgameState.board
    gameState.isAIGame = newgameState.isAIGame
    if (watcherState.human === gameState.isAIGame)
      watcherState.human = Cell.EMPTY;
    gameState.moveHistory = newgameState.moveHistory
    gameState.messages = processMessages(newgameState.messages)
    gameState.searchDepth = newgameState.searchDepth
    gameState.moveSuggestion = newgameState.moveSuggestion
    gameState.searchFunction = newgameState.searchFunction
    gameState.moveFunction = newgameState.moveFunction
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
      isAIGame: gameState.isAIGame.toString(),
      searchFunction: gameState.searchFunction,
      moveFunction: gameState.moveFunction,
      searchDepth: gameState.searchDepth.toString(),
      moveSuggestion: gameState.moveSuggestion.toString(),
      moveHistory: gameState.moveHistory.join(','),
      board_grid: gameState.board.grid.join(','),
      board_width: gameState.board.width.toString(),
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
    const query = (localStorage.getItem('gomoku-watcher-T0') ?? makeGameStateQuery())
      .replace(/isHumanGame=(false|true|\d)/, 'isAIGame=0') // fix old cache version
    if (!query || query.length <= 0)
      return console.debug('No T0.')
    fetch(`http://${window.location.hostname}:9012/debug-action?action=load-game-state&${query}`)
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
      if (!fetchIsAvailable.value)
        return false
      const resp = await fetch(`http://${window.location.hostname}:9012/gameState?silent`)
      if (resp.status != 200)
        throw Error('STATUS NOT 200')
      const data = await resp.json()
      if (checkResponse(data, resp))
        updateGameState(data)
    } catch {
      console.warn('server offline')
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
    overlay: {
      disabled: overlayDisabled,
      messages: overlayMessages,
      layers: overlayLayers
    },
    highlight: {
      get: () => highlightCircle.value,
      set: setHighlightCircle
    },
    fetchIsAvailable: {
      get: () => fetchIsAvailable.value,
      set: (value: boolean) => fetchIsAvailable.value = !!value
    }
  }
})
