#include "AI.hpp"

// AI_evaluate.cpp  (or replace the existing evaluate() body in AI.cpp)

/*
    Score table — tuned for a 5-in-a-row game (Gomoku / Gomoku-like).
    Positive = good for white, negative = good for black.

    Threat tiers:
      WIN        — five in a row, game over
      OPEN_4     — four with both ends free: opponent cannot stop two threats at once
      CLOSED_4   — four with one end free: forces the opponent's response but only one threat
      OPEN_3     — three with both ends free: next move creates an unstoppable open-4
      CLOSED_3   — three with one end free: weak, opponent can block
      OPEN_2     — two with both ends free: development, positional value
      CLOSED_2   — two with one end free: minimal value
*/
static constexpr float S_WIN       = 100000.f;
static constexpr float S_OPEN_4    =   9000.f;
static constexpr float S_CLOSED_4  =   4500.f;
static constexpr float S_OPEN_3    =   1500.f;
static constexpr float S_CLOSED_3  =    200.f;
static constexpr float S_OPEN_2    =     50.f;
static constexpr float S_CLOSED_2  =     10.f;

/*
    Determine how many ends of a NodeCellRow are "open" (i.e. extend into
    empty space rather than being blocked by an enemy stone or the board edge).

    A NodeCellRow's `prev` pointer points to the run that *precedes* it along
    the same axis-direction, and `next` points to the run that follows it.
    If that neighbour's type is EMPTY the end is open; if it is the opposite
    colour (or nullptr = board edge) the end is blocked.
*/
static int countOpenEnds(const NodeCellRow &cr) {
    int open = 0;
    if (cr.prev && cr.prev->type == Cell::EMPTY) ++open;
    if (cr.next && cr.next->type == Cell::EMPTY) ++open;
    return open;   // 0, 1, or 2
}

/*
    Map (size, openEnds) → score for one directional run.
    Returns 0 for sizes outside [2,4] since 5+ is already caught by isVictory()
    and 1 has no threat value.
*/
static float scoreRun(int size, int openEnds) {
    if (openEnds == 0) return 0.f;   // fully blocked — no potential
    switch (size) {
        case 4: return openEnds == 2 ? S_OPEN_4   : S_CLOSED_4;
        case 3: return openEnds == 2 ? S_OPEN_3   : S_CLOSED_3;
        case 2: return openEnds == 2 ? S_OPEN_2   : S_CLOSED_2;
        default: return 0.f;
    }
}

/*
    Walk every NodeCellRow produced by GridTraversal.
    Each NodeCellRow is one directional run of same-coloured stones on one axis.
    We accumulate scores for white and black separately, then return
    white_score - black_score  (+ = white advantage, - = black advantage).

    Important: GridTraversal creates one NodeCellRow per *distinct* run segment,
    so the same physical group of stones appears once per axis direction it was
    measured along (up to 4 axes). This is correct — a group that is open along
    the horizontal AND along a diagonal really does threaten in both directions.

    We skip:
      - EMPTY runs (no stones)
      - Runs of size 1 (no threat)
      - Runs of size >= 5 (already handled as a win by isVictory)

    The `score` field on NodeCellRow tracks the longest uninterrupted sub-run
    (resets across dead cells). We use this instead of raw `size` when the board
    has "holes" introduced by capture mechanics, so the score stays conservative.
*/
float AI::evaluate2(const Board &board, int16_t depth, Cell winningPlayer) {
    // Terminal: win/loss — penalise deeper wins slightly so the AI prefers
    // shorter paths to victory (and longer paths to defeat).
    if (winningPlayer == Cell::WHITE) return  S_WIN + depth;
    if (winningPlayer == Cell::BLACK) return -S_WIN - depth;

    // Edge case: board is empty or last move is invalid
    if (board.lastMove == Board::FIRSTMOVE || board.lastMove >= board.grid.size)
        return 0.f;

    const GridTraversal      &gt    = board.grid.nodes();
    const std::deque<NodeCellRow> &runs = gt.getCellRowsGarbage();

    float white = 0.f;
    float black = 0.f;

    for (const NodeCellRow &cr : runs) {
        if (cr.type == Cell::EMPTY) continue;

        // Use the streak score (longest clean sub-run) as the effective size.
        // This is more accurate than raw `size` when dead/captured cells split
        // a run internally. Fall back to `size` if score was never incremented.
        int effectiveSize = (cr.score > 0) ? static_cast<int>(cr.score)
                                           : static_cast<int>(cr.size);
        if (effectiveSize < 2 || effectiveSize >= 5) continue;

        int   openEnds = countOpenEnds(cr);
        float s        = scoreRun(effectiveSize, openEnds);
        if (s == 0.f) continue;

        if (cr.type == Cell::WHITE) white += s;
        else                        black += s;
    }

    // Tempo bonus: the side that just moved gets a tiny nudge so that
    // otherwise-equal positions prefer "my turn" over "opponent's turn".
    // isBlackToPlay == true means black just *finished* moving (white to play).
    constexpr float TEMPO = 15.f;
    if ( board.isBlackToPlay) white += TEMPO;   // white about to move
    else                       black += TEMPO;   // black about to move

    return white - black;
}