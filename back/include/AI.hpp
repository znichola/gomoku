#pragma once

#include <vector>
#include <string>

#include "TranspositionTable.hpp"
#include "Board.hpp"

namespace AI {
    enum class SearchFunction {
        MINMAX,
        MINMAX_JETESTE,
        NEGAMAX,
        ALPHABETA_NEGAMAX, ALPHABETA_NEGAMAX_TT
    };

    struct Eval {
        float black = 0;
        float white = 0;

        Eval operator-(const Eval& other) const;
        Eval operator+(const Eval& other) const;
        Eval operator*(const Eval& other) const;

        Eval& operator+=(const Eval& other);

        Eval operator*(float scale) const;
        Eval operator-(float scale) const;
        Eval operator+(float scale) const;
    };

    Eval operator-(float scale, const Eval& other);
    Eval operator+(float scale, const Eval& other);
    Eval operator*(float scale, const Eval& other);

    inline std::ostream &operator<<(std::ostream &os, const Eval &eval) {
        os << "[" << eval.black << ", " << eval.white << "]";
        return os;
    }

    struct EvalGroups {
        Eval open;
        Eval half;
    };

    Eval operator*(float scale, const Eval& e);

    inline int16_t maxDepth = 3;
    inline std::vector<int> nodeVisitCounter;
    inline TranspositionTable tt;

    // MinMax and varians

    float minMax(const Board &board, int16_t depth, bool maximizingPlayer);
    float negaMax(const Board &board, int16_t depth, float color);
    float alphaBetaNegaMax(const Board &board, int16_t depth, float a, float b, float color);
    float alphaBetaNegaMaxTT(const Board &board, int16_t depth, float a, float b, float color);

    unsigned play(const Board &board, bool isWhite, SearchFunction sf);
    unsigned findBestMove(const Board &board, bool isWhite, SearchFunction sf);
    float evaluate(const Board &board, int16_t depth, Cell winningPlayer);

    std::set<unsigned>getCandidateMoves(const Grid &grid);
    std::vector<unsigned>getOrderedCandidateMoves(const Grid &grid, unsigned bestMove, const Cell color);
    Eval countGroupsOf(const Board &board, int size);
    EvalGroups countOpenGroupsOf(const Board &board, int size);
    bool tryApplyTTBounds(uint64_t hash, int depth, float &alpha, float &beta, float &score, unsigned &bestMove);

    float minMax_jeteste1(const Board &board, int16_t depth, bool isBlackToPlay, int16_t level = 0);
    std::set<unsigned>getCandidateMoves_jeteste1(const Grid &grid, Cell color, int16_t level); 
};
