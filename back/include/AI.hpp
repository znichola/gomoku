#pragma once

#include <vector>
#include <string>
#include <future>
#include <thread>

#include "TranspositionTable.hpp"
#include "Board.hpp"
#include "AI_Eval.hpp"

#define __CHRONO_LABEL(name) MQ << "[CHRONO] " << #name << " took "

#define CHRONO_START(name) auto name = std::chrono::high_resolution_clock::now();
#define CHRONO_STOP(name) auto end ## name = std::chrono::high_resolution_clock::now();
#define CHRONO_PRINT(name) { \
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end ## name - name).count(); \
    if (us < 1'000) { __CHRONO_LABEL(name) << us << " µs\n"; } \
    else if (us < 1'000'000) { __CHRONO_LABEL(name) << us / 1'000.0 << " ms\n"; } \
    else { __CHRONO_LABEL(name) << us / 1'000'000.0 << " s\n"; } }
#define CHRONO_END(name) CHRONO_STOP(name) CHRONO_PRINT(name)

namespace AI {
    enum class SearchFunction {
        MINMAX,
        NEGAMAX,
        ALPHABETA_NEGAMAX, ALPHABETA_NEGAMAX_TT
    };

    enum class MoveFunction {
        CANDIDATE_MOVES,
        CANDIDATE_MOVES_2
    };

    inline int16_t maxDepth = 3;

    inline SearchFunction searchFunction = SearchFunction::ALPHABETA_NEGAMAX_TT;
    inline MoveFunction moveFunction = MoveFunction::CANDIDATE_MOVES_2;

    inline std::vector<int> nodeVisitCounter;
    inline std::vector<int> nodeEvalCounter;
    inline TranspositionTable tt;
    inline int64_t lastMoveMicros = -1; // duration of the last AI::play() call, for the UI timer

    // MinMax and varians

    float minMax(const Board &board, int16_t depth, bool isBlackToPlay, std::stop_token st);
    float negaMax(const Board &board, int16_t depth, float color, std::stop_token st);
    float alphaBetaNegaMax(const Board &board, int16_t depth, float a, float b, float color, std::stop_token st);
    float alphaBetaNegaMaxTT(const Board &board, int16_t depth, float a, float b, float color, std::stop_token st);

    unsigned play(const Board &board, bool isWhite);
    unsigned findBestMove(const Board &board, bool isWhite, std::stop_token st);
    unsigned findForcedMove(const Board &board, Cell myColor);
    float evaluate(const Board &board, int16_t depth, Cell winningPlayer);

    float mainSearch(const Board &board, float color, std::stop_token st);
    std::vector<unsigned>mainCandidateMoves(const Board &board, unsigned bestMove, float color, int depth);

    std::set<unsigned>getCandidateMoves(const Grid &grid);
    std::vector<unsigned>getOrderedCandidateMoves(const Grid &grid, unsigned bestMove, const Cell color);
    std::vector<unsigned>getOrderedCandidateMoves2(const Board &board, unsigned bestMove, float, int depth);
    Eval countGroupsOf(const Board &board, int size);

    // twos/threes/fours/captures computed in a single pass over the board's line segments,
    // instead of one full board scan per statistic.
    struct BoardStats {
        EvalGroups twos, threes, fours;
        Eval captures;
    };
    BoardStats gatherBoardStats(const Board &board);

    bool tryApplyTTBounds(uint64_t hash, int depth, float &alpha, float &beta, float &score, unsigned &bestMove);
};
