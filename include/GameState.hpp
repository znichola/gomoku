#pragma once

#include <string>

#include "Board.hpp"

struct GameState {
    bool isHumanGame = false;
    int movesPlayed = 0;
    Board board;

    std::string serialize() const;
    bool doMove(unsigned id);
};
