#pragma once

#include <vector>
#include <string>

#include "TranspositionTable.hpp"
#include "Board.hpp"
#include "AI_Eval.hpp"

namespace AI {
    enum class SearchFunction {
        MINMAX,
        MINMAX_JETESTE,
        NEGAMAX,
        ALPHABETA_NEGAMAX, ALPHABETA_NEGAMAX_TT
    };

    enum class MoveFunction {
        CANDIDATE_MOVES,
        CANDIDATE_MOVES_2,
        JETEST
    };

    inline int16_t maxDepth = 3;

    inline SearchFunction searchFunction = SearchFunction::ALPHABETA_NEGAMAX_TT;
    inline MoveFunction moveFunction = MoveFunction::CANDIDATE_MOVES_2;

    inline std::vector<int> nodeVisitCounter;
    inline TranspositionTable tt;

    // MinMax and varians

    float minMax(const Board &board, int16_t depth, bool maximizingPlayer);
    float negaMax(const Board &board, int16_t depth, float color);
    float alphaBetaNegaMax(const Board &board, int16_t depth, float a, float b, float color);
    float alphaBetaNegaMaxTT(const Board &board, int16_t depth, float a, float b, float color);

    unsigned play(const Board &board, bool isWhite);
    unsigned findBestMove(const Board &board, bool isWhite);
    float evaluate(const Board &board, int16_t depth, Cell winningPlayer);

    float mainSearch(const Board &board, float color);
    std::vector<unsigned>mainCandidateMoves(const Board &board, unsigned bestMove, float color, int depth);

    std::set<unsigned>getCandidateMoves(const Grid &grid);
    std::vector<unsigned>getOrderedCandidateMoves(const Grid &grid, unsigned bestMove, const Cell color);
    std::vector<unsigned>getOrderedCandidateMoves2(const Board &board, unsigned bestMove, float, int depth);
    Eval countGroupsOf(const Board &board, int size);
    Eval countCaptures(const Board &board);
    EvalGroups countOpenGroupsOf(const Board &board, int size);

    bool tryApplyTTBounds(uint64_t hash, int depth, float &alpha, float &beta, float &score, unsigned &bestMove);

    float minMax_jeteste1(const Board &board, int16_t depth, bool isBlackToPlay, int16_t level = 0);
    std::set<unsigned>getCandidateMoves_jeteste1(const Grid &grid, Cell color, int16_t level); 
};
