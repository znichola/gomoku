enum Cell { EMPTY=0, BLACK=1, WHITE=2 };

type Board = {
    grid: Cell[];
    boardDimentions: number;
    blackCaptured: number;
    whiteCaptured: number;
    isBlackToPlay: false;
}

type GameState = Board & {
    isHumanGame: boolean;
    movesPlayed: number[];
    board: Board;
}

export { Cell }
export type { Board, GameState };