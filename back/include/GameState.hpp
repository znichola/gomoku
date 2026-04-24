#pragma once

#include <string>

#include "AI.hpp"
#include "Board.hpp"

struct GameState {
    Cell isAIGame = Cell::EMPTY;
    Cell moveSuggestion = Cell::EMPTY; // Empty: non, Black: black, White: White, OUTSIDE: both
    AI::SearchFunction searchFunction = AI::SearchFunction::ALPHABETA_NEGAMAX_TT;
    Board board;
    std::vector<unsigned> moveHistory;

    std::string serialize() const;
    bool playMove(unsigned id);
    Cell askAI2Play();
    void reset();

    // debug
    void setReset();
    bool makeDoubleTree();

    void reload(const std::vector<Cell>& grid,
        unsigned blackCaptured,
        unsigned whiteCaptured,
        bool isBlackToPlay,
        unsigned width,
        const std::vector<unsigned>& moveHistory,
        Cell isAIGame);
};
