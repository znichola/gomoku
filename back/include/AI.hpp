#pragma once

#include <vector>

#include "Board.hpp"

namespace AI {
    inline int depth = 1;
    /* Searched through the board to a depth */
    unsigned minMax(const Board &board, int depth, bool maximizingPlayer);

    
    /* Returns a score for each potential move on the board (limmited to a square around played pieces) */
    std::vector<std::pair<unsigned, double>>heuristic(const Board &board);

    unsigned play(const Board &board, unsigned lastMove);
};
