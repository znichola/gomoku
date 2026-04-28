#include <iostream>
#include <limits>
#include <numeric>

#include "AI.hpp"
#include "MessageQueue.hpp"
#include <iomanip>

enum class Player {WHITE, BLACK};

static constexpr float INF = std::numeric_limits<float>::infinity();
static constexpr float WIN = 100000.0F; // Then endgame in x is maxDepth - depth

// https://en.wikipedia.org/wiki/Minimax#Pseudocode
unsigned AI::play(const Board &board, bool isWhite, SearchFunction sf) {
    unsigned move = findBestMove(board, isWhite, sf);
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
float AI::alphaBetaNegaMaxTT(const Board &board, int16_t depth, float a, float b, float color) {
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
    for (auto move : getOrderedCandidateMoves(board.grid, bestMove, color == -1.0f ? Cell::BLACK : Cell::WHITE)) {
        Board newBoard(board);
        if (newBoard.playMove(move) == false) continue;

        float score = -alphaBetaNegaMaxTT(newBoard, depth-1, -b, -a, -color);
        if (score > value) { value = score; bestMove = move; }
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
float AI::alphaBetaNegaMax(const Board &board, int16_t depth, float a, float b, float color) {
    nodeVisitCounter[depth] += 1;

    Cell victory = board.isVictory();
    if (depth == 0 || victory != Cell::EMPTY) {
        return color * evaluate(board, depth, victory);
    }

    float value = -INF;
    for (auto move : getOrderedCandidateMoves(board.grid, Board::FIRSTMOVE, color == -1.0f ? Cell::BLACK : Cell::WHITE)) {
        Board newBoard(board);
        if (newBoard.playMove(move) == false) continue;

        float child = -alphaBetaNegaMax(newBoard, depth-1, -b, -a, -color);
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
unsigned AI::findBestMove(const Board &board, bool isWhite, SearchFunction sf) {
    float bestScore = isWhite ? -INF : INF;
    unsigned bestMove = Board::FIRSTMOVE;
    AI::nodeVisitCounter.assign(AI::maxDepth + 1, 0);
    tt.newSearch();
    for (auto move : getCandidateMoves_jeteste1(board.grid, isWhite ? Cell::WHITE : Cell::BLACK, 0)) {
        Board newBoard(board);
        if (newBoard.playMove(move) == false) continue;
        float score = 0;
        switch (sf)
        {
        case SearchFunction::MINMAX:
            score = minMax(newBoard, AI::maxDepth, !isWhite);
            break;
        case SearchFunction::MINMAX_JETESTE:
            score = minMax_jeteste1(newBoard, AI::maxDepth, !isWhite);
            break;
        case SearchFunction::NEGAMAX:
            score = negaMax(newBoard, AI::maxDepth, isWhite ? 1 : -1);
            break;
        case SearchFunction::ALPHABETA_NEGAMAX:
            score = alphaBetaNegaMax(newBoard, AI::maxDepth, -INF, INF, isWhite ? 1 : -1);
            break;
        case SearchFunction::ALPHABETA_NEGAMAX_TT:
            score = alphaBetaNegaMaxTT(newBoard, AI::maxDepth, -INF, INF, isWhite ? 1 : -1);
            break;
        }
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << score;
        ENABLE_LOG MBQ(move, oss.str()); DISABLE_LOG
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

    // 1 for the side to play, 0 for the waiting side
    Eval active  = { static_cast<float>( board.isBlackToPlay),
                     static_cast<float>(!board.isBlackToPlay) };
    Eval passive = 1.0f - active;

    EvalGroups twos   = countOpenGroupsOf(board, 2);
    EvalGroups threes = countOpenGroupsOf(board, 3);
    EvalGroups fours  = countOpenGroupsOf(board, 4);

    Eval captures = {static_cast<float>(board.blackCaptured), static_cast<float>(board.whiteCaptured)};
    Eval possibleCaptures = countCaptures(board);

    Eval eval = fours.open  * (active * 1000.0f + passive * 900.0f)
              + fours.half  * (active *  950.0f + passive * 400.0f)
              + threes.open * (active *  700.0f + passive * 600.0f)
              + threes.half * (active *  200.0f + passive * 90.0f)
              + twos.open   * (active *   10.0f + passive * 6.0f)
              + twos.half   * (active *    8.5f + passive * 3.2f)
              + (possibleCaptures * 2 + captures) / 10 * (active * 1000.0f + passive * 700.0f)
              + active * 1.2f; // move advantage

    MQ << "Evaluate " << eval.white - eval.black
       << "\n" << (board.isBlackToPlay ? "black" : "white") << " to play" 
       << "\n black:" << eval.black << " white:" << eval.white
       << "\ntwos open:" << twos.open << "  half:" << twos.half
       << "\nthrees open:" << threes.open << "  half:" << threes.half
       << "\nfours open:" << fours.open << "  half:" << fours.half
       << "\npossible captures" << possibleCaptures;
       ;

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

AI::EvalGroups AI::countOpenGroupsOf(const Board &board, int size) {
    const auto &nodes = board.grid.nodes().getCellRowsGarbage();

    EvalGroups eg;
    for (const NodeCellRow &n : nodes) {
        if (n.size == 1 
                && n.type == Cell::EMPTY
                && n.prev && n.next
                && (n.next->type == Cell::BLACK || n.next->type == Cell::WHITE)
                && n.next->type == n.prev->type
                && (n.next->size + n.prev->size) == size) {
            if (n.next->type == Cell::BLACK) eg.half.black++; else eg.half.white++;
        }
        if (n.size != size) continue;
        if (n.type != Cell::BLACK && n.type != Cell::WHITE) continue;

        bool openL = n.prev && n.prev->type == Cell::EMPTY;
        bool openR = n.next && n.next->type == Cell::EMPTY;
        int openEnds = (int)openL + (int)openR;

        if (openEnds == 0) continue; // closed, skip

        Eval& target = (openEnds == 2) ? eg.open : eg.half;
        if (n.type == Cell::BLACK) target.black++; else target.white++;
    }
    return eg;
}

AI::Eval AI::countCaptures(const Board &board) {
    const auto &nodes = board.grid.nodes().getCellRowsGarbage();

    Eval eval;
    for (const NodeCellRow &n : nodes) {
        if (n.size != 2) continue;
        if (n.type != Cell::BLACK && n.type != Cell::WHITE) continue;

        Cell opponent = (n.type == Cell::BLACK) ? Cell::WHITE : Cell::BLACK;

        bool canCapture = n.prev && n.next && (
            (n.next->type == opponent && n.prev->type == Cell::EMPTY) ||
            (n.next->type == Cell::EMPTY && n.prev->type == opponent)
        );

        if (n.type == Cell::WHITE) eval.black += int(canCapture); else eval.white += int(canCapture);
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

std::vector<unsigned> AI::getOrderedCandidateMoves(const Grid &grid, unsigned bestMove, const Cell color) {
    const GridTraversal &gt = grid.nodes();
    const std::vector<AdjacentNode> &gridCR = gt.getGridCellRow();

    (void) bestMove;

    std::set<unsigned> moves = getCandidateMoves(grid);

    struct Compare {
        bool operator()(const std::pair<unsigned, unsigned>& a,
                        const std::pair<unsigned, unsigned>& b) const {
            if (a.second != b.second)
                return a.second > b.second; // score décroissant
            return a.first < b.first;   // tie-break sur id
        }
    };
    std::set<std::pair<unsigned, unsigned>, Compare> stones;

    for (std::set<unsigned>::iterator id = moves.begin(); id != moves.end(); ++id) {
        const AdjacentNode &adj = gridCR[*id];
        unsigned score = 0;
        for (unsigned ext = 0; ext < 4; ++ext) {
            if (!adj[ext]) continue;
            const NodeCellRow &cr = *adj[ext];
            if (cr.prev && cr.prev->type == color && std::abs(cr.prev->originId - *id) <= 1 && cr.size + cr.prev->size > 5)
                score += 5 + cr.size + cr.prev->size;
            if (cr.next && cr.next->type == color && std::abs(cr.next->originId - *id) <= 1 && cr.size + cr.next->size > 5) {
                score += 5 + cr.size + cr.next->size;
            }
        }
        stones.insert({*id, score});
    }

    std::vector<unsigned> result;

    for (const auto& [id, score] : stones) {
        if (score > 1)
            result.push_back(id);
    }
    if (result.empty()) {
        result.insert(result.end(), moves.begin(), moves.end());
    }

    return result;
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


// Eval

AI::Eval AI::Eval::operator+(const Eval& other) const {
    return { black + other.black, white + other.white };
}

AI::Eval AI::Eval::operator-(const Eval& other) const {
    return { black - other.black, white - other.white };
}

AI::Eval AI::Eval::operator*(const Eval& other) const {
    return { black * other.black, white * other.white };
}

AI::Eval AI::Eval::operator/(const Eval& other) const {
    return { black / other.black, white / other.white };
}


AI::Eval& AI::Eval::operator+=(const Eval& other) {
    black += other.black;
    white += other.white;
    return *this;
}


AI::Eval AI::Eval::operator-(float scale) const {
    return { black - scale, white - scale };
}

AI::Eval AI::Eval::operator+(float scale) const {
    return { black + scale, white + scale };
}

AI::Eval AI::Eval::operator*(float scale) const {
    return { black * scale, white * scale };
}

AI::Eval AI::Eval::operator/(float scale) const {
    return { black / scale, white / scale };
}


AI::Eval AI::operator-(float scale, const Eval& other) {
    return { scale - other.black, scale - other.white };
}

AI::Eval AI::operator+(float scale, const Eval& other) {
    return { scale + other.black, scale + other.white };
}

AI::Eval AI::operator*(float scale, const Eval& other) {
    return { scale * other.black, scale * other.white };
}

AI::Eval AI::operator/(float scale, const Eval& other) {
    return { scale / other.black, scale / other.white };
}