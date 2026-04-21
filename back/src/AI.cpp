#include <iostream>
#include <limits>

#include "AI.hpp"
#include "MessageQueue.hpp"

enum class Player {WHITE, BLACK};

static constexpr float INF = std::numeric_limits<float>::infinity();

// https://en.wikipedia.org/wiki/Minimax#Pseudocode
unsigned AI::play(const Board &board, bool isWhite) {
    unsigned move = bestMove(board, isWhite);
    if (move == Board::FIRSTMOVE) return 180;
    return move;
}

/*
    MinMax algo - minimizing possible losses
    https://en.wikipedia.org/wiki/Minimax#Pseudocode

    maximisingPlayer is a flag to toggle searching for
    lowest score or highest score possible
    black likes low scores, so it should be called with a false here

    TODO see negamax for simpler version of the function, and alpha-beta pruning for more optimised one
*/
float AI::minMax(const Board &board, int depth, bool maximizingPlayer) {
    Cell victory = board.isVictory();
    if (depth == 0 || victory != Cell::EMPTY) {
        return evaluate(board, victory); // evaluation/heuristic is only run for terminal nodes
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
unsigned AI::bestMove(const Board &board, bool isWhite) {
    float bestScore = isWhite ? -INF : INF;
    unsigned bestMove = Board::FIRSTMOVE;
    for (auto move : getCandidateMoves(board.grid)) {
        Board newBoard(board);
        if (newBoard.playMove(move) == false) continue;
        float score = minMax(newBoard, AI::depth, !isWhite);
        if (isWhite ? score > bestScore : score < bestScore) { // if white we look for highest score, if black we look for lowest
            bestMove = move;
            bestScore = score;
        }

    }
    if (bestMove == Board::FIRSTMOVE) {
        MQ << "[AI] No best move found";
        std::cout << "[AI] No best move found\n";
    }
    return bestMove;
}

/*
    This is an evaluation of the entire board state. 
    return a score of the position: - for black and + for white
    This function is only called at terminal nodes of the tree (see subject p5)
*/
float AI::evaluate(const Board &board, Cell winningPlayer) {
    if (winningPlayer == Cell::WHITE) return INF;
    if (winningPlayer == Cell::BLACK) return -INF;
    if (board.lastMove == Board::FIRSTMOVE || board.lastMove >= board.grid.size()) {
        return 0;
    }
    // A basic heuristic for now, all can be thrown away.
    int blackCount = 0;
    int whiteCount = 0;
    for (size_t id = 0; id < board.grid.size(); id++) {
        auto piece = board.grid[id];
        if (piece == Cell::BLACK) blackCount++;
        if (piece == Cell::WHITE) whiteCount++;
    }
    return whiteCount - blackCount; // If black has more pieces, it's better for them
}

std::set<unsigned> AI::getCandidateMoves(const Grid &grid) {
    std::set<unsigned> cm;
    for (size_t id = 0; id < grid.size(); id++) {
        if (grid.grid[id] == Cell::EMPTY) continue;
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
