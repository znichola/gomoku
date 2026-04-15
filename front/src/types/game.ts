enum Cell { EMPTY=0, BLACK=1, WHITE=2 };

type Board = {
    grid: Cell[];
    boardDimension: number;
    blackCaptured: number;
    whiteCaptured: number;
    isBlackToPlay: boolean;
}

type GameState = {
    isAIGame: Cell;
    moveHistory: number[];
    board: Board | null;
    messages: string[];
}

export { Cell }
export type { Board, GameState };
