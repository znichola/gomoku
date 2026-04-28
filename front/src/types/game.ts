export enum Cell { EMPTY=0, BLACK=1, WHITE=2, OUTSIDE=3 }

type SearchFunction =
  | 'MINMAX'
  | 'NEGAMAX'
  | 'ALPHABETA_NEGAMAX'
  | 'ALPHABETA_NEGAMAX_TT'

type MoveSuggestion =
  | 'off'
  | 'black'
  | 'white'
  | 'both'

export interface Board {
  grid: Cell[]
  width: number
  blackCaptured: number
  whiteCaptured: number
  isBlackToPlay: boolean
  winner: Cell
}

export interface GameState {
  searchDepth: number
  searchFunction: SearchFunction
  moveSuggestion: MoveSuggestion
  isAIGame: Cell
  moveHistory: number[]
  board: Board | null
  messages: string[]
}

export interface OverlayLayer {
  name: string
  color: string
}

export interface OverlayMessage {
  id: number
  msg: string
  layer?: string
  group?: OverlayLayer
}
