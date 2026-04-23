enum Cell { EMPTY=0, BLACK=1, WHITE=2, OUTSIDE=3 }

type Board = {
    grid: Cell[]
    width: number
    blackCaptured: number
    whiteCaptured: number
    isBlackToPlay: boolean
    winner: Cell
}

type GameState = {
    isAIGame: Cell
    moveHistory: number[]
    board: Board | null
    messages: string[]
}

export { Cell }
export type { Board, GameState }
