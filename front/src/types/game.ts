export enum Cell { EMPTY=0, BLACK=1, WHITE=2, OUTSIDE=3 }

export type SearchFunction =
  | 'MINMAX'
  | 'NEGAMAX'
  | 'MINMAX_JETESTE'
  | 'ALPHABETA_NEGAMAX'
  | 'ALPHABETA_NEGAMAX_TT'

export type MoveSuggestion =
  | 'off'
  | 'black'
  | 'white'
  | 'both'

export type MoveFunction =
  | 'CANDIDATE_MOVES'
  | 'CANDIDATE_MOVES_2'
  | 'JETEST'

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
  moveFunction : MoveFunction
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
