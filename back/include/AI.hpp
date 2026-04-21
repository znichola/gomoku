#pragma once

#include <vector>
#include <string>

#include "Board.hpp"

namespace AI {
    typedef float htype;

    inline int depth = 1;

    unsigned bestMove(const Board &board, bool isWhite);
    float minMax(const Board &board, int depth, bool maximizingPlayer);
    float evaluate(const Board &board, Cell winningPlayer);

    /* Returns a score for each potential move on the board (limmited to a square around played pieces) */
    std::vector<std::pair<unsigned, float>>heuristic(const Board &board);
    unsigned play(const Board &board, bool isWhite);

    std::set<unsigned>getCandidateMoves(const Grid &grid);
};
