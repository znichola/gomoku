#pragma once

#include <vector>
#include <string>

#include "TranspositionTable.hpp"
#include "Board.hpp"

namespace AI {
    enum class SearchFunction { MINMAX, NEGAMAX, ALPHABETA};

    inline int16_t maxDepth = 2;
    inline std::vector<int> nodeVisitCounter;
    inline TranspositionTable tt;

    unsigned bestMove(const Board &board, bool isWhite, SearchFunction sf);
    float minMax(const Board &board, int16_t depth, bool maximizingPlayer);
    float negaMax(const Board &board, int16_t depth, float color);
    float alphaBetaNegaMax(const Board &board, int16_t depth, float a, float b, float color);
    float evaluate(const Board &board, int16_t depth, Cell winningPlayer);

    /* Returns a score for each potential move on the board (limmited to a square around played pieces) */
    std::vector<std::pair<unsigned, float>>heuristic(const Board &board);
    unsigned play(const Board &board, bool isWhite);

    std::set<unsigned>getCandidateMoves(const Grid &grid);
    std::vector<unsigned>getOrderedCandidateMoves(const Grid &grid, unsigned bestMove);

    bool tryApplyTTBounds(uint64_t hash, int depth, float &alpha, float &beta, float &score, unsigned &bestMove);
};
