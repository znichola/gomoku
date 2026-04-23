#pragma once

#include <vector>
#include <string>

#include "Board.hpp"

namespace AI {
    enum class SearchFunction { MINMAX, NEGAMAX, ALPHABETA};

    inline int maxDepth = 2;
    inline std::vector<int> nodesExplored;

    unsigned bestMove(const Board &board, bool isWhite, SearchFunction sf);
    float minMax(const Board &board, int depth, bool maximizingPlayer);
    float negaMax(const Board &board, int depth, float color);
    float alphaBetaNegaMax(const Board &board, int depth, float a, float b, float color);
    float evaluate(const Board &board, int depth, Cell winningPlayer);

    /* Returns a score for each potential move on the board (limmited to a square around played pieces) */
    std::vector<std::pair<unsigned, float>>heuristic(const Board &board);
    unsigned play(const Board &board, bool isWhite);

    std::set<unsigned>getCandidateMoves(const Grid &grid);
    std::vector<unsigned>getOrderedCandidateMoves(const Grid &grid);
};
