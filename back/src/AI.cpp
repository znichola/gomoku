#include <iostream>
#include <limits>

#include "AI.hpp"
#include "MessageQueue.hpp"

enum class Player {WHITE, BLACK};

static constexpr float INF = std::numeric_limits<float>::infinity();
static constexpr float WIN = 100000.0F; // Then endgame in x is maxDepth - depth

// https://en.wikipedia.org/wiki/Minimax#Pseudocode
unsigned AI::play(const Board &board, bool isWhite) {
    unsigned move = bestMove(board, isWhite, SearchFunction::ALPHABETA);
    if (move == Board::FIRSTMOVE) return 180;
    return move;
}

/*
    AlphaBeta with NegaMax - branch pruning, minimise losses
    https://en.wikipedia.org/wiki/Negamax

    Much like negamax, as black pass color=-1 as white color=1
*/
float AI::alphaBetaNegaMax(const Board &board, int depth, float a, float b, float color) {
    Cell victory = board.isVictory();
    if (depth == 0 || victory != Cell::EMPTY) {
        return color * evaluate(board, depth, victory);
    }

    float value = -INF;
    for (auto move : getOrderedCandidateMoves(board.grid)) {
        Board newBoard(board);
        if (newBoard.playMove(move) == false) continue;
        value = std::max(value, -alphaBetaNegaMax(newBoard, depth -1, -b, -a, -color));
        a = std::max(a, value);
        if (a >= b) break;
    }
    return value;
}

/*
    NegaMax algo - alt for miniMax, minimise losses
    https://en.wikipedia.org/wiki/Negamax

    The color is used to invert the evaluation value.
    Evaluation always returns + for white and - for black.
    If black to play, call with color=-1
*/
float AI::negaMax(const Board &board, int depth, float color) {
    Cell victory = board.isVictory();
    if (depth == 0 || victory != Cell::EMPTY) {
        return color * evaluate(board, depth, victory);
    }
    float value = -INF;
    for (auto move : getCandidateMoves(board.grid)) {
        Board newBoard(board);
        if (newBoard.playMove(move) == false) continue;
        value = std::max(value, -negaMax(newBoard, depth-1, -color));
    }
    return value;
}

/*
    MinMax algo - minimizing possible losses
    https://en.wikipedia.org/wiki/Minimax#Pseudocode

    maximisingPlayer is a flag to toggle searching for
    lowest score or highest score possible.
    black likes low scores, so it should be called with a false here.

    TODO see negamax for simpler version of the function, and alpha-beta pruning for more optimised one
*/
float AI::minMax(const Board &board, int depth, bool maximizingPlayer) {
    Cell victory = board.isVictory();
    if (depth == 0 || victory != Cell::EMPTY) {
        return evaluate(board, depth, victory); // evaluation/heuristic is only run for terminal nodes
    }
    if (maximizingPlayer) {
        float best = -INF;
        for (auto move : getCandidateMoves(board.grid)) {
            Board newBoard(board); // Optimise, maybe with an undoMove, to remove the constructor/destructor in a loop
            if (newBoard.playMove(move) == false) continue; // skip illegal moves
            float score = minMax(newBoard, depth-1, false);
            best = std::max(score, best);
        }
        return best;
    } else {
        float best = INF;
        for (auto move : getCandidateMoves(board.grid)) {
            Board newBoard(board);
            if (newBoard.playMove(move) == false) continue;
            float score = minMax(newBoard, depth-1, true);
            best = std::min(score, best);
        }
        return best;
    }
}


/*
    Choosing the best move by using minMax
*/
unsigned AI::bestMove(const Board &board, bool isWhite, SearchFunction sf) {
    float bestScore = isWhite ? -INF : INF;
    unsigned bestMove = Board::FIRSTMOVE;
    for (auto move : getCandidateMoves(board.grid)) {
        Board newBoard(board);
        if (newBoard.playMove(move) == false) continue;
        float score = 0;
        switch (sf)
        {
        case SearchFunction::MINMAX:
            score = minMax(newBoard, AI::maxDepth, !isWhite);
            break;
        case SearchFunction::NEGAMAX:
            score = negaMax(newBoard, AI::maxDepth, isWhite ? 1 : -1);
            break;
        case SearchFunction::ALPHABETA:
            score = alphaBetaNegaMax(newBoard, AI::maxDepth, -INF, INF, isWhite ? 1 : -1);
        }
        ENABLE_LOG MBQ(move, std::to_string(score)); DISABLE_LOG
        if (isWhite ? score > bestScore : score < bestScore) { // if white we look for highest score, if black we look for lowest
            bestMove = move;
            bestScore = score;
        }
    }
    if (bestMove == Board::FIRSTMOVE) {
        MQ << "[AI] No best move found";
        COUT << "[AI] No best move found\n";
        DISABLE_LOG
    }
    return bestMove;
}

/*
    This is an evaluation of the entire board state. 
    return a score of the position: - for black and + for white
    This function is only called at terminal nodes of the tree (see subject p5)
*/
float AI::evaluate(const Board &board, int depth, Cell winningPlayer) {
    if (winningPlayer == Cell::WHITE) return WIN + depth;
    if (winningPlayer == Cell::BLACK) return -WIN - depth;
    if (board.lastMove == Board::FIRSTMOVE || board.lastMove >= board.grid.size) {
        return 0;
    }
    // A basic heuristic for now, all can be thrown away.
    int blackCount = 0;
    int whiteCount = 0;
    for (size_t id = 0; id < board.grid.size; id++) {
        auto piece = board.grid[id];
        if (piece == Cell::BLACK) blackCount++;
        if (piece == Cell::WHITE) whiteCount++;
    }
    return whiteCount - blackCount; // If black has more pieces, it's better for them
}

std::set<unsigned> AI::getCandidateMoves(const Grid &grid) {
    std::set<unsigned> cm;
    for (size_t id = 0; id < grid.size; id++) {
        if (grid[id] == Cell::EMPTY) continue;
        Vector2D v = grid.idToVec(id);
        for (const Vector2D &e: EXTREMITIES) {
            Vector2D nv = v + e;
            if (!grid.isInside(nv)) continue;
            unsigned nid = grid.vecToId(nv);
            if (grid[nid] == Cell::EMPTY)
                cm.insert(nid);
        }
    }
    return cm;
}

std::vector<unsigned> AI::getOrderedCandidateMoves(const Grid &grid) {
    auto moves = getCandidateMoves(grid);
    return std::vector<unsigned>(moves.begin(), moves.end());
}