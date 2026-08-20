#include <iostream>
#include <limits>
#include <numeric>

#include "AI.hpp"
#include "Server.hpp"
#include "MessageQueue.hpp"
#include <iomanip>
#include <algorithm>

enum class Player {WHITE, BLACK};

static constexpr float INF = std::numeric_limits<float>::infinity();
static constexpr float WIN = 100000.0F; // Then endgame in x is maxDepth - depth

// https://en.wikipedia.org/wiki/Minimax#Pseudocode
unsigned AI::play(const Board &board, bool isWhite) {
    using namespace std::chrono_literals;

    int64_t us = 0;

    std::promise<int> pMove;
    std::future<int> fMove = pMove.get_future();
    DISABLE_LOG
    std::jthread tMove([&](std::stop_token st){
        auto start = std::chrono::high_resolution_clock::now();
        unsigned cid = findBestMove(board, isWhite, st);
        auto end = std::chrono::high_resolution_clock::now();
        us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        pMove.set_value_at_thread_exit(cid);
    });
    std::cout << "[";

    std::future_status status;
    int i = 0;
    do {
        status = fMove.wait_for(50ms);
        if (i++ > 60 || g_interrupted.load(std::memory_order_relaxed)) { // 20 fps = ~1sec
            std::cout << "k" << std::flush;
            tMove.request_stop();
        } else {
            std::cout << "." << std::flush;
        }
    } while (status != std::future_status::ready);

    ENABLE_LOG
    if (us < 1'000) {
        COUT << "] " << us << " µs\n";
        MQ   << "[AI] Move took " << us << " µs\n";
    } else if (us < 1'000'000) {
        COUT << "] " << us / 1'000.0 << " ms\n";
        MQ   << "[AI] Move took " << us / 1'000.0 << " ms\n";
    } else {
        COUT << "] " << us / 1'000'000.0 << " s\n";
        MQ   << "[AI] Move took " << us / 1'000'000.0 << " s\n";
    }
    AI::lastMoveMicros = us;
    unsigned move = fMove.get();
    if (move == Board::FIRSTMOVE) {
        COUT << "[AI] No best move found\n";
        return 180;
    }
    return move;
}

/*
    AlphaBeta with NegaMax - branch pruning, minimise losses
    https://en.wikipedia.org/wiki/Negamax
    https://en.wikipedia.org/wiki/Negamax#Negamax_with_alpha_beta_pruning_and_transposition_tables

    RETURN VALUE: Perspective-relative

    alpha & beta reprisent the search window for the position score.

    alpha is the lower bound, beta the upper,
    the true score is some where inbetween.

    Much like negamax, as black pass color=-1 as white color=1.
*/
float AI::alphaBetaNegaMaxTT(const Board &board, int16_t depth, float a, float b, float color, std::stop_token st) {
    if (st.stop_requested()) {
        return 0;
    }
    nodeVisitCounter[depth] += 1;
    uint64_t hash = board.grid.getHash();

    // Lookup previous position, either tighten search window (adjust a b bestMove)
    // or return exact score if it's already stored
    float ttScore = 0;
    unsigned bestMove = Board::FIRSTMOVE;
    if (tryApplyTTBounds(hash, depth, a, b, ttScore, bestMove))
        return ttScore;

    Cell victory = board.winner;
    if (depth == 0 || victory != Cell::EMPTY) {
        return color * evaluate(board, depth, victory);
    }

    float origA = a;

    float value = -INF;
    for (auto move : mainCandidateMoves(board, bestMove, -color, depth)) {
        Board newBoard(board);
        if (newBoard.playMove(move) == false) continue;

        float score = -alphaBetaNegaMaxTT(newBoard, depth-1, -b, -a, -color, st);
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

    RETURN VALUE: Perspective-relative

    Positive scores favor the player to move, negative scores favor the opponent.
*/
float AI::alphaBetaNegaMax(const Board &board, int16_t depth, float a, float b, float color, std::stop_token st) {
    if (st.stop_requested()) {
        return 0;
    }
    nodeVisitCounter[depth] += 1;

    Cell victory = board.winner;
    if (depth == 0 || victory != Cell::EMPTY) {
        return color * evaluate(board, depth, victory);
    }

    float value = -INF;
    for (auto move : mainCandidateMoves(board, Board::FIRSTMOVE, -color, depth)) {
        Board newBoard(board);
        if (newBoard.playMove(move) == false) continue;

        float child = -alphaBetaNegaMax(newBoard, depth-1, -b, -a, -color, st);
        if (child > value) value = child;
        a = std::max(a, value);
        if (a >= b) break;
    }
    return value;
}



/*
    NegaMax algo - alt for miniMax, minimise losses
    https://en.wikipedia.org/wiki/Negamax

    RETURN VALUE: Perspective-relative
s
    The color is used to invert the evaluation value.
    Evaluation always returns + for white and - for black.
    If black to play, call with color=-1
*/
float AI::negaMax(const Board &board, int16_t depth, float color, std::stop_token st) {
    if (st.stop_requested()) {
        return 0;
    }
    nodeVisitCounter[depth] += 1;
    Cell victory = board.winner;
    if (depth == 0 || victory != Cell::EMPTY) {
        return color * evaluate(board, depth, victory);
    }
    float value = -INF;
    for (auto move : mainCandidateMoves(board, Board::FIRSTMOVE, -color, depth)) {
        Board newBoard(board);
        if (newBoard.playMove(move) == false) continue;
        value = std::max(value, -negaMax(newBoard, depth-1, -color, st));
    }
    return value;
}

/*
    MinMax algo - minimizing possible losses
    https://en.wikipedia.org/wiki/Minimax#Pseudocode

    RETURN VALUE: Absolute perspective

    maximisingPlayer is a flag to toggle searching for
    lowest score or highest score possible.
    black likes low scores, so it should be called with a false here.

    TODO see negamax for simpler version of the function, and alpha-beta pruning for more optimised one
*/
float AI::minMax(const Board &board, int16_t depth, bool isBlackToPlay, std::stop_token st) {
	if (st.stop_requested()) {
		return 0;
	}
	nodeVisitCounter[depth] += 1;
	if (depth == 0 || board.winner != Cell::EMPTY) {
		return evaluate(board, depth, board.winner); // evaluation/heuristic is only run for terminal nodes
	}
	
	float best = isBlackToPlay ? -INF : INF;
	//auto minmax = (isBlackToPlay) ? std::max<float> : std::min<float>;
	
	for (auto move : mainCandidateMoves(board, Board::FIRSTMOVE, isBlackToPlay ? -1 : 1, depth)) {
		if (!board.isValidMove(move)) continue; // skip illegal moves
		float score = minMax(Board(board, move), depth-1, !isBlackToPlay, st);
		best = (isBlackToPlay) ? std::max(score, best) : std::min(score, best);
	}
	return best;
}

/*
    Cheap tactical safety net, checked before the heuristic search even starts: does ANY
    candidate move win outright for `myColor` right now? If not, is there a cell where the
    OPPONENT would win outright if they got to play there next? A handful of direct
    isVictory()/getWinningLineColor() checks (one Board copy per raw candidate, no
    recursion) is a few orders of magnitude cheaper than a full search, and unlike the
    heuristic search below it can't be fooled: it's not an approximation, it's just asking
    the same rules engine that ends the game whether this exact move ends it.

    This exists because the heuristic search (localTacticalScore ordering, MAX_MOVES-wide
    candidate lists, alpha-beta pruning trusting that ordering) is fundamentally an
    approximation, and in some board configurations it can fail to notice or properly
    prioritize exactly this kind of forced, unambiguous tactic - regardless of how deep it
    searches. Confirmed against two real lost games (see the "half-open four" and
    "diagonal four" regression tests in test/tactics.py) where the AI had a forced block
    available and searched right past it.
*/
unsigned AI::findForcedMove(const Board &board, Cell myColor) {
    const Cell opponent = (myColor == Cell::BLACK) ? Cell::WHITE : Cell::BLACK;
    std::set<unsigned> candidates = getCandidateMoves(board.grid);

    for (unsigned move : candidates) {
        Board myTry(board);
        if (!myTry.playMove(move)) continue;
        if (myTry.winner == myColor) return move;
    }

    std::vector<unsigned> oppWinningMoves;
    for (unsigned move : candidates) {
        if (!board.isValidMove(move)) continue; // has to be legal for me to actually play here
        Board oppTry(board);
        oppTry.grid.set(move, opponent);
        if (oppTry.grid.getWinningLineColorNear(move) == opponent) oppWinningMoves.push_back(move);
    }

    if (oppWinningMoves.size() == 1) return oppWinningMoves.front();

    if (oppWinningMoves.size() > 1) {
        for (unsigned move : candidates) {
            if (!board.isValidMove(move)) continue;
            if (board.grid.detectCaptures(move, myColor) <= 0) continue; // only captures can kill a shared stone

            Board myTry(board);
            if (!myTry.playMove(move)) continue;

            bool stillForked = false;
            for (unsigned oppMove : oppWinningMoves) {
                Board oppTry(myTry);
                oppTry.grid.set(oppMove, opponent);
                if (oppTry.grid.getWinningLineColorNear(oppMove) == opponent) { stillForked = true; break; }
            }
            if (!stillForked) return move;
        }
        // No capture rescues it: the position is genuinely lost to this fork. Don't return
        // a block that doesn't work - fall through to the real search.
    }

    return Board::FIRSTMOVE;
}

/*
    Choosing the best move by using iterative deepening: search depth 1, then 2, then 3...
    up to the configured AI::maxDepth, always keeping the move found by the LAST FULLY
    COMPLETED depth as the answer.

    Why: a plain fixed-depth search has no valid move to fall back on if the time budget
    (the stop_token, see AI::play) runs out mid-search - alphaBetaNegaMaxTT returns 0 for
    any node it had to abandon, and that 0 can silently bubble up and corrupt the score of
    whichever root move was being explored when time ran out. Iterative deepening sidesteps
    this: each depth is only ever accepted once it has finished normally, so a timeout just
    means "we stop at the deepest depth we had time to finish", never a corrupted pick.

    It also happens to make deeper searches cheaper: the best move from depth d-1 is passed
    back in as the move-ordering hint for depth d (mainCandidateMoves' bestMove param), and
    the transposition table carries over across depths (single tt.newSearch() for the whole
    call), so alpha-beta gets much tighter bounds earlier at each new depth.
*/
unsigned AI::findBestMove(const Board &board, bool isWhite, std::stop_token st) {
    Cell myColor = isWhite ? Cell::WHITE : Cell::BLACK;
    unsigned forced = findForcedMove(board, myColor);
    if (forced != Board::FIRSTMOVE) {
        ENABLE_LOG MQ << "[AI] forced move (immediate win, or forced block): " << forced << "\n"; DISABLE_LOG
        return forced;
    }

    const int16_t targetDepth = AI::maxDepth;
    unsigned bestMove = Board::FIRSTMOVE;
    int16_t lastDepthRun = 0;
    tt.newSearch();

    for (int16_t d = 1; d <= targetDepth; ++d) {
        if (st.stop_requested())
            break;

        tableGridTraversal.clear();

        AI::maxDepth = d; // drives evaluate()'s mate-distance scoring and mainSearch's depth for this iteration
        AI::nodeVisitCounter.assign(d + 1, 0);
        AI::nodeEvalCounter.assign(d + 1, 0);

        float bestScoreThisDepth = -INF;
        unsigned bestMoveThisDepth = Board::FIRSTMOVE;
        bool interrupted = false;

        auto candidateMoves = mainCandidateMoves(board, bestMove, isWhite ? 1 : -1, d);
        for (auto move : candidateMoves) {
            if (st.stop_requested()) { interrupted = true; break; }
            Board newBoard(board);
            if (newBoard.playMove(move) == false) continue;
            float score = mainSearch(newBoard, isWhite ? 1 : -1, st);
            if (st.stop_requested()) { interrupted = true; break; } // score may be 0-contaminated, don't trust it
            ENABLE_LOG MBL("findBestMove", move, score); DISABLE_LOG
            if (score >= bestScoreThisDepth) {
                bestMoveThisDepth = move;
                bestScoreThisDepth = score;
            }
        }

        if (interrupted || bestMoveThisDepth == Board::FIRSTMOVE) {
            ENABLE_LOG MQ << "[AI] depth " << d << " interrupted, keeping depth " << lastDepthRun << "'s move\n"; DISABLE_LOG
            break;
        }

        bestMove = bestMoveThisDepth;
        lastDepthRun = d;
        ENABLE_LOG MQ << "[AI] depth " << d << " complete: move " << bestMove << " (" << bestScoreThisDepth << ")\n"; DISABLE_LOG
    }

    AI::maxDepth = targetDepth; // restore the configured depth (used by evaluate()/UI outside of a search)

    ENABLE_LOG
    if (bestMove == Board::FIRSTMOVE) {
        MQ << "[AI] No best move found";
        auto candidateMoves = mainCandidateMoves(board, Board::FIRSTMOVE, isWhite ? 1 : -1, 1);
        if (candidateMoves.empty()) {
            MQ << " (no candidates)";
        }  else {
            MQ << " (first candirate move played instead)";
            bestMove = *candidateMoves.begin();
        }
    }
    MQ << "[AI] reached depth " << lastDepthRun << "/" << targetDepth << ", explored "
       << std::accumulate(nodeVisitCounter.begin(), nodeVisitCounter.end(), 0) << " nodes\n"
       << "and evaluated " << std::accumulate(nodeEvalCounter.begin(), nodeEvalCounter.end(), 0) << " positions\n"
       << [](){
        std::stringstream ss;
        for (int i = static_cast<int>(nodeVisitCounter.size()) - 1, last = 1; 0 <= i; i--) {
            ss << "Ply " << (nodeVisitCounter.size() - i) << ": " << nodeVisitCounter[i] << " nodes (x"
                << (nodeVisitCounter[i] * 100 / std::max(1, last)) / 100.0 << ")\n";
            last = nodeVisitCounter[i];
        }
        return ss.str();
    }();
    DISABLE_LOG
    return bestMove;
}

/*
    This is an evaluation of the entire board state. 
    return a score of the position: - for black and + for white
    This function is only called at terminal nodes of the tree (see subject p5)
*/
float AI::evaluate(const Board &board, int16_t depth, Cell winningPlayer) {
    if (depth >= 0 && static_cast<size_t>(depth) < nodeEvalCounter.size())
        nodeEvalCounter[depth] += 1;
    if (winningPlayer == Cell::WHITE) return WIN + (maxDepth - depth);
    if (winningPlayer == Cell::BLACK) return -WIN - (maxDepth - depth);
    if (board.lastMove == Board::FIRSTMOVE || board.lastMove >= board.grid.size) {
        return 0;
    }
    const TTEntry* e = tt.probe(board.grid.getHash());
    if (e) return e->score;

    // 1 for the side to play, 0 for the waiting side
    Eval active  = { static_cast<float>( board.isBlackToPlay),
                     static_cast<float>(!board.isBlackToPlay) };
    Eval passive = 1.0f - active;

    BoardStats stats = gatherBoardStats(board);
    EvalGroups &twos   = stats.twos;
    EvalGroups &threes = stats.threes;
    EvalGroups &fours  = stats.fours;

    Eval &possibleCaptures = stats.captures;

    auto captureDanger = [](unsigned capturedStones) -> float {
        float pairs = static_cast<float>(capturedStones) / 2.0f;
        return pairs * pairs * 150.0f;
    };
    Eval captureBanked = {captureDanger(board.blackCaptured), captureDanger(board.whiteCaptured)};

    Eval eval = fours.open  * (active * 5000.0f + passive * 4200.0f)
              + fours.half  * (active *  950.0f + passive * 400.0f)
              + threes.open * (active *  700.0f + passive * 600.0f)
              + threes.half * (active *  200.0f + passive * 90.0f)
              + twos.open   * (active *   10.0f + passive * 6.0f)
              + twos.half   * (active *    8.5f + passive * 3.2f)
              + possibleCaptures * (active * 200.0f + passive * 140.0f)
              + captureBanked
              + active * 1.2f; // move advantage

    const float res = eval.white - eval.black;
    MQ << "Evaluate " << res
       << "\n" << (board.isBlackToPlay ? "black" : "white") << " to play" 
       << "\n black:" << eval.black << " white:" << eval.white
       << "\ntwos open:" << twos.open << "  half:" << twos.half
       << "\nthrees open:" << threes.open << "  half:" << threes.half
       << "\nfours open:" << fours.open << "  half:" << fours.half
       << "\npossible captures" << possibleCaptures;
       ;

    tt.store(board.grid.getHash(), res, 0, -1, Bound::ONEOFF);
    return res; // + is good for white, - good for black
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

/*
    Single pass over the board's line segments (NodeCellRow) that used to be
    4 separate full scans (countOpenGroupsOf(2/3/4) + countCaptures). Each
    segment can only match one of: a capturable pair, an empty gap between two
    same-color runs (half-open Nth), or a contiguous same-color run of 2/3/4 -
    so the three checks below are mutually exclusive per node.
*/
AI::BoardStats AI::gatherBoardStats(const Board &board) {
    const auto &nodes = board.grid.nodes().getCellRowsGarbage();

    BoardStats stats;

    auto groupsFor = [&](unsigned size) -> EvalGroups* {
        switch (size) {
            case 2: return &stats.twos;
            case 3: return &stats.threes;
            case 4: return &stats.fours;
            default: return nullptr;
        }
    };

    for (const NodeCellRow &n : nodes) {
        if (n.size == 2 && (n.type == Cell::BLACK || n.type == Cell::WHITE)) {
            Cell opponent = (n.type == Cell::BLACK) ? Cell::WHITE : Cell::BLACK;
            bool canCapture = n.prev && n.next && (
                (n.next->type == opponent && n.prev->type == Cell::EMPTY) ||
                (n.next->type == Cell::EMPTY && n.prev->type == opponent)
            );
            if (n.type == Cell::WHITE) stats.captures.black += int(canCapture);
            else stats.captures.white += int(canCapture);
        }

        if (n.size == 1
                && n.type == Cell::EMPTY
                && n.prev && n.next
                && (n.next->type == Cell::BLACK || n.next->type == Cell::WHITE)
                && n.next->type == n.prev->type) {
            if (EvalGroups* eg = groupsFor(n.next->size + n.prev->size)) {
                if (n.next->type == Cell::BLACK) eg->half.black++; else eg->half.white++;
            }
            continue;
        }

        if (n.type != Cell::BLACK && n.type != Cell::WHITE) continue;
        EvalGroups* eg = groupsFor(n.size);
        if (!eg) continue;

        bool openL = n.prev && n.prev->type == Cell::EMPTY;
        bool openR = n.next && n.next->type == Cell::EMPTY;
        int openEnds = (int)openL + (int)openR;

        if (openEnds == 0) continue; // closed, skip

        Eval& target = (openEnds == 2) ? eg->open : eg->half;
        if (n.type == Cell::BLACK) target.black++; else target.white++;
    }
    return stats;
}

std::vector<unsigned> AI::mainCandidateMoves(
    const Board &board, unsigned bestMove, float color, int depth
) {
    switch (moveFunction) {
    case MoveFunction::CANDIDATE_MOVES_2:
        return getOrderedCandidateMoves2(board, bestMove, color, depth);
    }
    std::runtime_error("Must select valid Move function");
}

/*
    Wrapper function to select the search algo used

    Normalizes all results to perspective-relative
    + is for the searching player - for the opponent
*/
float AI::mainSearch(const Board &board, float color, std::stop_token st) {
    bool isWhite = color == 1;
    switch (searchFunction) {
        case SearchFunction::MINMAX:
            return color * minMax(board, AI::maxDepth, !isWhite, st);
        case SearchFunction::NEGAMAX:
            return negaMax(board, AI::maxDepth, color, st);
        case SearchFunction::ALPHABETA_NEGAMAX:
            return alphaBetaNegaMax(board, AI::maxDepth, -INF, INF, color, st);
        case SearchFunction::ALPHABETA_NEGAMAX_TT:
            return alphaBetaNegaMaxTT(board, AI::maxDepth, -INF, INF, color, st);
    }
    throw std::runtime_error("Must select valid Search function");
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

// TODO unused, but could be interesting
// Normalize scores to [0,1] then measure how "spread out" they are
// Low entropy = one dominant move = danger/forced
// High entropy = many good moves = safe, prune aggressively
inline float scoreEntropy(const std::vector<std::pair<unsigned, float>> &scoredMoves) {
    if (scoredMoves.size() <= 1) return 0.f;

    float minS = scoredMoves.back().second;
    float maxS = scoredMoves.front().second;
    float range = maxS - minS;

    if (range < 1e-6f) return 1.f; // all moves equal = maximally safe

    // Softmax-style: how much probability mass is on the top move?
    // A sharp peak = low entropy = danger
    constexpr float temperature = 10.f; // tune to your score scale
    float sum = 0.f;
    for (auto &[move, s] : scoredMoves)
        sum += std::exp((s - maxS) / temperature);

    // Fraction of mass on best move: close to 1 = forced, close to 1/N = safe
    float topFraction = 1.f / sum; // exp(0)/sum

    // Convert to a danger signal in [0,1]: 1 = fully forced, 0 = all moves equal
    return topFraction; // rename to "forcedness" if you like
}

/*
    Local (no board copy, no GridTraversal rebuild) estimate of how good playing `color` at
    `id` looks. For each of the 4 line axes through `id` (using the 8 EXTREMITIES directions,
    opposite pairs forming an axis), walks outward in both directions counting consecutive
    same-color stones - once for `color` (offense) and once for the opponent (defense/blocking).
    Longer and more open runs score higher, roughly mirroring evaluate()'s own ordering (open
    four > half four > open three > ...). Also checks for an immediate capture via
    Grid::detectCaptures, which - like the rest of this function - reads the board directly at
    fixed offsets around `id` without needing the move actually played.

    This replaces move ordering that used to copy the board and run the full evaluate() (4
    board-wide scans + a fresh GridTraversal construction) for every single candidate at every
    node of the search tree. An earlier version of this function tried to reuse the cached
    GridTraversal's line segments instead of walking the grid directly, gating on the flanking
    segment's size - that was wrong: a segment's size is the length of the WHOLE contiguous
    empty run id belongs to, not id's distance from the stone it's touching, so it silently
    skipped axes where id was directly touching a critical run (e.g. failed to find an obvious
    forced block). Walking the grid directly has no such ambiguity.
*/
static float localTacticalScore(const Board &board, unsigned id, Cell color) {
    const Grid &grid = board.grid;
    const Cell opponent = (color == Cell::BLACK) ? Cell::WHITE : Cell::BLACK;
    const Vector2D origin = grid.idToVec(id);

    auto runWeight = [](int size, bool open) -> float {
        static constexpr float OPEN[] = {0, 1, 8, 60, 1500, 5000};
        static constexpr float HALF[] = {0, 1, 3, 20, 150, 5000};
        size = std::min(size, 5);
        return open ? OPEN[size] : HALF[size];
    };

    auto walk = [&](int dirIdx, Cell who) -> std::pair<int, int> {
        const Vector2D &dir = EXTREMITIES[dirIdx];
        int stones = 0;
        Vector2D p = origin + dir;
        while (grid.isInside(p) && grid[grid.vecToId(p)] == who) {
            ++stones;
            p = p + dir;
        }
        int room = 0;
        while (grid.isInside(p) && grid[grid.vecToId(p)] == Cell::EMPTY) {
            ++room;
            p = p + dir;
        }
        return {stones, room};
    };

    static constexpr int AXES[4][2] = {{0, 4}, {1, 5}, {2, 6}, {3, 7}}; // opposite EXTREMITIES pairs
    float score = 0;
    for (const auto &axis : AXES) {
        for (Cell who : {color, opponent}) {
            auto [stonesPos, roomPos] = walk(axis[0], who);
            auto [stonesNeg, roomNeg] = walk(axis[1], who);
            int connected = 1 + stonesPos + stonesNeg; // the stone we'd place, plus both directions
            if (connected <= 1) continue;

            int maxReach = connected + roomPos + roomNeg;
            if (maxReach < 5) continue;

            float w = runWeight(connected, roomPos > 0 && roomNeg > 0);
            score += (who == color) ? w : w * 0.85f; // blocking matters, but slightly less than extending
        }
    }

    if (grid.detectCaptures(id, color) > 0) score += 250;

    for (const Vector2D &dir : EXTREMITIES) {
        const Vector2D neighbor = origin + dir;
        if (!grid.isInside(neighbor) || grid[grid.vecToId(neighbor)] != color) continue;

        const Vector2D back = origin - dir;
        const Vector2D front = neighbor + dir;
        const Cell backC = grid.isInside(back) ? grid[grid.vecToId(back)] : Cell::OUTSIDE;
        const Cell frontC = grid.isInside(front) ? grid[grid.vecToId(front)] : Cell::OUTSIDE;
        if (backC == color || frontC == color) continue; // part of a longer run, never capturable

        const bool backEnemy = backC == opponent, backEmpty = backC == Cell::EMPTY;
        const bool frontEnemy = frontC == opponent, frontEmpty = frontC == Cell::EMPTY;
        if ((backEnemy && frontEmpty) || (backEmpty && frontEnemy)) score -= 100;
    }

    // Defensive bonus: does this move DEFUSE an existing vulnerable pair of mine (already
    // on the board, from an earlier move) by extending it to a run of 3+? A pair flanked by
    // one enemy stone and one empty cell is one legal move away from being captured; if
    // `id` is that empty flank, playing there removes the vulnerability outright (a run of
    // 3+ can never be captured, per the rules), which is worth more than the ordinary
    // alignment-extension score alone - without this, the AI has no reason to prefer
    // defusing an active threat over any other similarly-scored extension, and a pair can
    // sit exposed for turns until the opponent finally cashes it in (confirmed against a
    // real lost game: white had 4 spare turns to play the exact cell that would have
    // defused pair 84-85 flanked by black at 83, played elsewhere every time, and black
    // captured it - one of 5 pairs lost that game).
    for (const Vector2D &dir : EXTREMITIES) {
        const Vector2D p1 = origin + dir;
        const Vector2D p2 = origin + dir * 2;
        const Vector2D p3 = origin + dir * 3;
        if (!grid.isInside(p1) || !grid.isInside(p2) || !grid.isInside(p3)) continue;
        if (grid[grid.vecToId(p1)] == color && grid[grid.vecToId(p2)] == color &&
            grid[grid.vecToId(p3)] == opponent) {
            score += 250;
        }
    }

    return score;
}

std::vector<unsigned> AI::getOrderedCandidateMoves2(const Board &board, unsigned bestMove, float color, int depth) {
    std::set<unsigned> moves = getCandidateMoves(board.grid);
    const Cell cColor = color == -1 ? Cell::BLACK : Cell::WHITE;

    std::vector<std::pair<unsigned, float>> scoredMoves;
    scoredMoves.reserve(moves.size());

    for (auto move : moves) {
        if (bestMove != Board::FIRSTMOVE && bestMove == move) {
            scoredMoves.push_back({move, INF});
            continue;
        }
        if (board.grid.isDoubleThree(move, cColor)) continue;
        scoredMoves.push_back({move, localTacticalScore(board, move, cColor)});
    }

    constexpr size_t MAX_MOVES = 3;

	// Trie seulement les 3 premiers
	size_t count = std::min(MAX_MOVES, scoredMoves.size());
	std::partial_sort(scoredMoves.begin(), scoredMoves.begin() + count, scoredMoves.end(),
		[](const auto& a, const auto& b) {
			return a.second > b.second;  // Ordre décroissant
		});

    size_t width = std::min(MAX_MOVES, scoredMoves.size());

    std::vector<unsigned> orderedMoves;
    orderedMoves.reserve(width);

    scoredMoves.resize(width);
    for (auto [move, score] : scoredMoves) {
        if (depth == -1) {
            ENABLE_LOG MBL("getOrderedCandidateMoves2", move, score); DISABLE_LOG
        }
        orderedMoves.push_back(move);
    }
    return orderedMoves;
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
        case Bound::ONEOFF:
            break;
    }

    if (alpha >= beta) {
        score = e->score;
        return true;
    }

    return false;
}
