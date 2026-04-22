#pragma once

#include <string>

#include "Board.hpp"

struct GameState {
    Cell isAIGame = Cell::EMPTY;
    Board board;
    std::vector<unsigned> moveHistory;

    std::string serialize() const;
    bool playMove(unsigned id);
    bool askAI2Play();
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
