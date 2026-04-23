#include <iostream>
#include <limits>
#include <numeric>

#include "AI.hpp"
#include "MessageQueue.hpp"

enum class Player {WHITE, BLACK};

static constexpr float INF = std::numeric_limits<float>::infinity();
static constexpr float WIN = 100000.0F; // Then endgame in x is maxDepth - depth

// https://en.wikipedia.org/wiki/Minimax#Pseudocode
unsigned AI::play(const Board &board, bool isWhite) {
    unsigned move = bestMove(board, isWhite, SearchFunction::ALPHABETA_NEDAMAX_NOTT);
    if (move == Board::FIRSTMOVE) return 180;
    return move;
}

/*
    AlphaBeta with NegaMax - branch pruning, minimise losses
    https://en.wikipedia.org/wiki/Negamax
    https://en.wikipedia.org/wiki/Negamax#Negamax_with_alpha_beta_pruning_and_transposition_tables


    alpha & beta reprisent the search window for the position score.

    alpha is the lower bound, beta the upper,
    the true score is some where inbetween.

    Much like negamax, as black pass color=-1 as white color=1.
*/
float AI::alphaBetaNegaMax(const Board &board, int16_t depth, float a, float b, float color) {
    nodeVisitCounter[depth] += 1;
    uint64_t hash = board.grid.getHash();

    // Lookup previous position, either tighten search window (adjust a b bestMove)
    // or return exact score if it's already stored
    float ttScore = 0;
    unsigned bestMove = Board::FIRSTMOVE;
    if (tryApplyTTBounds(hash, depth, a, b, ttScore, bestMove))
        return ttScore;

    Cell victory = board.isVictory();
    if (depth == 0 || victory != Cell::EMPTY) {
        return color * evaluate(board, depth, victory);
    }

    float origA = a;

    float value = -INF;
    for (auto move : getOrderedCandidateMoves(board.grid, bestMove)) {
        Board newBoard(board);
        if (newBoard.playMove(move) == false) continue;

        float child = -alphaBetaNegaMax(newBoard, depth-1, -b, -a, -color);
        if (child > value) { value = child; bestMove = move; }
        a = std::max(a, value);
        if (a >= b) break;
    }

    // Determine bound and store
    Bound bound = value <= origA ? Bound::UPPER
                : value >= b     ? Bound::LOWER
                                 : Bound::EXACT;
    tt.store(hash, value, depth, bestMove, bound);
    return value;
}

/*
    Alpha Beta - variant with no Transposition table
*/
float AI::alphaBetaNegaMaxNoTT(const Board &board, int16_t depth, float a, float b, float color) {
    nodeVisitCounter[depth] += 1;

    Cell victory = board.isVictory();
    if (depth == 0 || victory != Cell::EMPTY) {
        return color * evaluate(board, depth, victory);
    }

    float value = -INF;
    for (auto move : getOrderedCandidateMoves(board.grid, Board::FIRSTMOVE)) {
        Board newBoard(board);
        if (newBoard.playMove(move) == false) continue;

        float child = -alphaBetaNegaMaxNoTT(newBoard, depth-1, -b, -a, -color);
        if (child > value) value = child;
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
float AI::negaMax(const Board &board, int16_t depth, float color) {
    nodeVisitCounter[depth] += 1;
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
float AI::minMax(const Board &board, int16_t depth, bool maximizingPlayer) {
    nodeVisitCounter[depth] += 1;
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
    AI::nodeVisitCounter.assign(AI::maxDepth + 1, 0);
    tt.newSearch();
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
        case SearchFunction::ALPHABETA_NEDAMAX:
            score = alphaBetaNegaMax(newBoard, AI::maxDepth, -INF, INF, isWhite ? 1 : -1);
            break;
        case SearchFunction::ALPHABETA_NEDAMAX_NOTT:
            score = alphaBetaNegaMaxNoTT(newBoard, AI::maxDepth, -INF, INF, isWhite ? 1 : -1);
            break;
        }
        ENABLE_LOG MBQ(move, std::to_string(score)); DISABLE_LOG
        if (isWhite ? score > bestScore : score < bestScore) { // if white we look for highest score, if black we look for lowest
            bestMove = move;
            bestScore = score;
        }
    }
    ENABLE_LOG
    if (bestMove == Board::FIRSTMOVE) {
        MQ << "[AI] No best move found";
        COUT << "[AI] No best move found\n";
    }
    MQ << "[AI] explored " << std::accumulate(nodeVisitCounter.begin(), nodeVisitCounter.end(), 0) << " nodes\n"
       << [](){
        std::stringstream ss;
        for (int i = static_cast<int>(nodeVisitCounter.size()) - 1; 0 <= i; i--) {
            ss << "Depth: " << maxDepth - i << " :: " << nodeVisitCounter[i] << " nodes\n";
        }
        return ss.str();}();
    DISABLE_LOG
    return bestMove;
}

/*
    This is an evaluation of the entire board state. 
    return a score of the position: - for black and + for white
    This function is only called at terminal nodes of the tree (see subject p5)
*/
float AI::evaluate(const Board &board, int16_t depth, Cell winningPlayer) {
    if (winningPlayer == Cell::WHITE) return WIN + depth;
    if (winningPlayer == Cell::BLACK) return -WIN - depth;
    if (board.lastMove == Board::FIRSTMOVE || board.lastMove >= board.grid.size) {
        return 0;
    }

    Eval tempo = {
        static_cast<float>(board.isBlackToPlay),
        static_cast<float>(!board.isBlackToPlay)
    };

    Eval eval;
    Eval twos = countGroupsOf(board, 2);
    Eval threes = countGroupsOf(board, 3);
    // Eval fours = countGroupsOf(board, 4);

    eval.black += twos.black * 1.1f + tempo.black * 0.2;
    eval.white += twos.white * 1.1f + tempo.white * 0.2;

    eval.black += threes.black * 0.2f + tempo.black * 0.1;
    eval.white += threes.white * 0.2f + tempo.white * 0.1;

    // eval.black += fours.black * 2;
    // eval.white += fours.white * 2;

    return eval.white - eval.black; // + is good for white, - good for black
}

AI::Eval AI::countGroupsOf(const Board &board, int size) {
    const GridTraversal &gt = board.grid.nodes();
    const std::deque<NodeCellRow> &nodes = gt.getCellRowsGarbage();

    Eval eval;
    for (const NodeCellRow &n : nodes) {
        if (n.size == size) {
            if (n.type ==Cell::BLACK) eval.black++;
            if (n.type ==Cell::WHITE) eval.white++;
        }
    }
    return eval;
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

std::vector<unsigned> AI::getOrderedCandidateMoves(const Grid &grid, unsigned bestMove) {
    auto moves = getCandidateMoves(grid);
    if (bestMove != Board::FIRSTMOVE && moves.contains(bestMove)) {
        moves.erase(bestMove);
        std::vector<unsigned> result{moves.begin(), moves.end()};
        result.insert(result.begin(), bestMove);
        return result;
    }
    return std::vector<unsigned>(moves.begin(), moves.end());
}

/*

*/
bool AI::tryApplyTTBounds(uint64_t hash, int depth, float &alpha, float &beta, float &score, unsigned &bestMove) {
    const TTEntry* e = tt.probe(hash);
    if (!e || e->depth < depth) return false;

    bestMove = e->move;
    switch (e->bound) {
        case Bound::EXACT:
            score = e->score;
            return true;
        case Bound::LOWER:
            alpha = std::max(alpha, e->score);
            break;
        case Bound::UPPER:
            beta = std::min(beta, e->score);
            break;
    }

    if (alpha >= beta) {
        score = e->score;
        return true;
    }

    return false;
}