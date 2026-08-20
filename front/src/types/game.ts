export enum Cell { EMPTY=0, BLACK=1, WHITE=2, OUTSIDE=3 }

export type SearchFunction =
  | 'MINMAX'
  | 'NEGAMAX'
  | 'ALPHABETA_NEGAMAX'
  | 'ALPHABETA_NEGAMAX_TT'

export type MoveSuggestion =
  | 'off'
  | 'black'
  | 'white'
  | 'both'

export type MoveFunction =
  | 'CANDIDATE_MOVES'

export interface Board {
  grid: Cell[]
  width: number
  blackCaptured: number
  whiteCaptured: number
  isBlackToPlay: boolean
  winner: Cell
  winByCaptures: boolean
  winByAlignment: boolean
  winningCells: number[]
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
  /** Duration of the last AI move search, in microseconds. -1 if no AI move has been played yet. */
  aiMoveMicros: number
  /** Wall-clock cutoff given to the backend's AI search, in milliseconds. */
  aiTimeBudgetMs: number
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
