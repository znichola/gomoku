#pragma once

#include <string>

#include "Board.hpp"

struct GameState {
    bool isHumanGame = false;
    Board board;
    std::vector<unsigned> moveHistory;

    std::string serialize() const;
    bool playMove(unsigned id);
    void reset();

    // debug
    bool makeDoubleTree();
};
