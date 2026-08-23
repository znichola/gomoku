#!/usr/bin/env python3
"""
Tactical regression tests for the Gomoku AI, run against a live backend via /analyze.

/analyze loads an arbitrary position into a throwaway GameState (not the live game),
lets the AI play one move from it, and returns the result - so this is safe to run
against a server you're also playing on manually (e.g. via `make dev`), it will not
touch your game's board or move history.

Usage:
    python3 test/tactics.py [--host localhost] [--port 9012] [--depth 8]

Each test sets up a hand-picked position with an unambiguous correct answer (block a
forced threat, take a free capture, extend to a near-win, ...) and checks the AI's
chosen move against it. These are not exhaustive - they're regression tests for
specific tactical blind spots found while debugging the AI's move ordering
(localTacticalScore in AI.cpp), meant to catch it if a future change reintroduces one.
"""
import argparse
import json
import sys
import urllib.request

WIDTH = 19
BLACK, WHITE = 1, 2


def analyze(host, port, grid_overrides, black_to_play, ai_color, depth,
            black_captured=0, white_captured=0, search_function=None):
    grid = [0] * (WIDTH * WIDTH)
    for idx, val in grid_overrides.items():
        grid[idx] = val
    params = (
        "board_grid=" + ",".join(map(str, grid)) +
        f"&board_width={WIDTH}"
        f"&board_blackCaptured={black_captured}"
        f"&board_whiteCaptured={white_captured}"
        f"&board_isBlackToPlay={str(black_to_play).lower()}"
        "&moveHistory="
        f"&isAIGame={ai_color}"
        f"&searchDepth={depth}"
    )
    if search_function:
        params += f"&searchFunction={search_function}"
    url = f"http://{host}:{port}/api/analyze?{params}"
    with urllib.request.urlopen(url, timeout=15) as r:
        return json.load(r)


# Coordinates: id = row * 19 + col. Row 9 (the middle row) starts at id 171,
# so ids 171..189 are cols 0..18 of that row - handy for building a straight line
# of stones without worrying about wrapping to the next/previous row.
TESTS = [
    {
        "name": "block a half-open four or lose next move",
        "grid": {175: WHITE, 176: BLACK, 177: BLACK, 178: BLACK, 179: BLACK},
        "black_to_play": False,
        "ai_color": WHITE,
        "expected": {180},
    },
    {
        "name": "take a free capture",
        "grid": {171: WHITE, 172: BLACK, 173: BLACK},
        "black_to_play": False,
        "ai_color": WHITE,
        "expected": {174},
    },
    {
        "name": "block an open three before it becomes an open four",
        "grid": {179: BLACK, 180: BLACK, 181: BLACK},
        "black_to_play": False,
        "ai_color": WHITE,
        "expected": {178, 182},
    },
    {
        "name": "extend own open three to an open four",
        "grid": {172: WHITE, 173: WHITE, 174: WHITE},
        "black_to_play": False,
        "ai_color": WHITE,
        "expected": {171, 175},
    },
    {
        # Diagonal equivalent of the first test - regression test for a real lost game
        # (searchDepth 8, ALPHABETA_NEGAMAX): a candidate-ordering call inside the
        # alpha-beta recursion used `color` instead of `-color` for the move being
        # chosen at that node (board reflects the OTHER player's move, so it's always
        # -color's turn there) - alpha-beta pruning trusts that ordering to be correct,
        # so it silently pruned away the branch that would have revealed black's forced
        # win instead of just being slower to find it. Only showed up past ~depth 7,
        # and identically on both alphaBetaNegaMax and alphaBetaNegaMaxTT.
        "name": "diagonal half-open four, block or lose (needs real depth to expose)",
        "grid": {160: WHITE, 142: BLACK, 124: BLACK, 106: BLACK, 88: BLACK, 144: WHITE, 162: WHITE},
        "black_to_play": False,
        "ai_color": WHITE,
        "expected": {70},
    },
]


def check_tt_pollution_regression(host, port, depth):
    """
    Regression test for a specific transposition-table bug: alphaBetaNegaMaxTT used to
    store (maxDepth - depth) - the ply from the root - as a TT entry's "depth", instead
    of `depth` itself (how many plies were actually searched below that position). Since
    AI::tt is a single global table shared by every /analyze call (never cleared between
    them), a position could pick up a stale, insufficiently-deep cached bound left behind
    by an earlier, unrelated search that happened to reach the same board hash, and trust
    it as if it reflected a real search - e.g. missing an opponent's forced winning reply
    entirely. This replays the exact sequence of positions that exposed it (see git
    history / conversation around this test file), ending on the one that used to fail.
    """
    core_black, core_white = [159, 158, 160, 161], [157]

    def play(extra_black, extra_white):
        grid = {m: BLACK for m in core_black + extra_black}
        grid.update({m: WHITE for m in core_white + extra_white})
        data = analyze(host, port, grid, black_to_play=False, ai_color=WHITE, depth=depth)
        return data["moveHistory"][-1] if data["moveHistory"] else None

    # Prime the shared TT with a handful of related-but-different searches first.
    play([], [])
    play([121], [])
    play([], [139])
    play([], [140])
    move = play([], [139, 140])  # this exact one used to come back wrong
    play([121], [139, 140])
    play([], [175])

    ok = move == 162
    print(f"[{'PASS' if ok else 'FAIL'}] TT staleness regression (block after priming the cache) "
          f"-> played {move}, expected 162")
    return ok


def check_self_capture_regression(host, port, depth):
    """
    Regression test for a real lost game: white repeatedly replayed stones into the same
    neighborhood, each time forming a fresh pair next to one of its own existing stones
    with the opponent's flank already in place - handing black 5 free pairs across one
    game (10 captured stones = an outright win by the capture rule). localTacticalScore's
    move ordering rewarded captures the mover makes but had no term at all for captures the
    mover was about to hand the opponent, so the AI kept walking back into the same trap.

    Minimal repro: black at 104, white at 84. Playing 64 would form a white pair (84,64)
    flanked by black at 104 (far side) and empty at 44 (near side) - one legal move away
    from black capturing it (the exact pattern from the real game, move #17).
    """
    grid = {104: BLACK, 84: WHITE}
    data = analyze(host, port, grid, black_to_play=False, ai_color=WHITE, depth=depth)
    move = data["moveHistory"][-1] if data["moveHistory"] else None

    ok = move != 64
    print(f"[{'PASS' if ok else 'FAIL'}] self-capture regression (don't hand the opponent a free pair) "
          f"-> played {move}, must not be 64")
    return ok


def check_defend_vulnerable_pair_regression(host, port, depth):
    """
    Regression test for a real lost game (searchFunction=MINMAX): white had an existing
    pair (84,85) flanked by black at 83 on one side and empty at 86 on the other - one
    legal move away from being captured - and spent 4 spare turns playing elsewhere before
    black finally captured it (one of 5 pairs lost that game, enough to lose outright via
    the 10-capture rule). localTacticalScore had a term for not CREATING a new vulnerable
    pair, but nothing rewarding DEFUSING an existing one by extending it to an uncapturable
    run of 3 - so the AI had no reason to prefer playing 86 over any other similarly-scored
    move, and let the threat sit until the opponent cashed it in.

    Minimal repro: black at 83, white at 84 and 85. Playing 86 extends the pair to a run of
    3 (83 is black, 84-85-86 would all be white), removing the vulnerability outright.
    """
    grid = {83: BLACK, 84: WHITE, 85: WHITE}
    data = analyze(host, port, grid, black_to_play=False, ai_color=WHITE, depth=depth)
    move = data["moveHistory"][-1] if data["moveHistory"] else None

    ok = move == 86
    print(f"[{'PASS' if ok else 'FAIL'}] defend vulnerable pair regression (defuse a capture threat by extending to 3) "
          f"-> played {move}, expected 86")
    return ok


def check_fork_no_false_block_regression(host, port, depth):
    """
    Regression test for findForcedMove: it used to return the FIRST cell where the
    opponent would win, as if finding one such cell always meant a single block settles
    it. For a clean open four (176-179, nothing capturable anywhere), BOTH ends (175 and
    180) independently let black win - blocking one is a false sense of security, since
    black just plays the other end next turn. There's no way to rescue this (any stone
    within the four capturable enough to matter would already make that end not count as
    a win at all, via the Endgame Capture rule - see the git history around this test for
    why an earlier version of it tried exactly that and was wrong), so the only correct
    behavior is to recognize the block doesn't work and let the real search run instead
    of confidently returning a move that provably doesn't save the game.

    Checked indirectly via the message log: findForcedMove short-circuiting logs "forced
    move" and returns without ever running the depth-by-depth search; falling through logs
    "reached depth X/Y, explored N nodes" instead.
    """
    grid = {176: BLACK, 177: BLACK, 178: BLACK, 179: BLACK}
    data = analyze(host, port, grid, black_to_play=False, ai_color=WHITE, depth=depth)
    messages = data.get("messages", [])

    forced = any("forced move" in m for m in messages)
    searched = any("reached depth" in m for m in messages)

    ok = not forced and searched
    print(f"[{'PASS' if ok else 'FAIL'}] fork non-rescue regression (open four doesn't get a false-confidence block) "
          f"-> forced_move_shortcut={forced}, real_search_ran={searched}")
    return ok


def check_minmax_board_state_regression(host, port, depth):
    """
    Regression test for searchFunction=MINMAX specifically: AI::minMax's recursive step
    used to build each child position via Board(board.grid, move) - a constructor that
    only copies the grid, silently resetting blackCaptured/whiteCaptured to 0 and
    isBlackToPlay to its default (true) at EVERY node. That meant every simulated move got
    played as BLACK regardless of whose turn it actually was, and all capture-count context
    was wiped throughout the whole search - MINMAX failed even a trivial "take a free
    capture" position because of it (a real lost game surfaced this: the user reported the
    AI playing badly, which traced back to this, not to a heuristic regression).

    Fixed by copying the whole board and using its normal playMove instead. This replays
    the two simplest tests from TESTS above but forcing searchFunction=MINMAX, since the
    rest of this file's tests all rely on the default (ALPHABETA_NEGAMAX_TT) and would not
    have caught a MINMAX-only bug.
    """
    ok = True

    data = analyze(host, port, {171: WHITE, 172: BLACK, 173: BLACK}, black_to_play=False,
                    ai_color=WHITE, depth=depth, search_function="MINMAX")
    move = data["moveHistory"][-1] if data["moveHistory"] else None
    this_ok = move == 174
    ok &= this_ok
    print(f"[{'PASS' if this_ok else 'FAIL'}] MINMAX board-state regression (take a free capture) "
          f"-> played {move}, expected 174")

    data = analyze(host, port, {175: WHITE, 176: BLACK, 177: BLACK, 178: BLACK, 179: BLACK},
                    black_to_play=False, ai_color=WHITE, depth=depth, search_function="MINMAX")
    move = data["moveHistory"][-1] if data["moveHistory"] else None
    this_ok = move == 180
    ok &= this_ok
    print(f"[{'PASS' if this_ok else 'FAIL'}] MINMAX board-state regression (block a half-open four) "
          f"-> played {move}, expected 180")

    return ok


def check_capture_count_hash_collision_regression(host, port, _depth):
    """
    Regression test for a hashing bug: Grid::getHash() only Zobrist-hashes stone
    positions - it knows nothing about blackCaptured/whiteCaptured or isBlackToPlay.
    AI::tt is a single table shared across the whole game (and, via /api/analyze,
    across unrelated positions too - see the TT staleness regression above), and every
    TranspositionTable::store()/probe() used to be keyed directly off that stone-only
    hash (computed ad hoc at each AI.cpp call site). So two totally different real game
    states that happen to share the same stone layout - but different capture counts,
    or different side to move - collided on the same TT slot and silently reused each
    other's cached score/bound, corrupting the search: since evaluate() weighs
    captureBanked by pairs^2 * 150 and picks its whole active/passive weight table off
    isBlackToPlay, the reused score could be wildly wrong for the actual position.
    Confirmed live: the "take a free capture" position below reliably stopped finding
    the capture after a handful of unrelated /analyze calls hit the same server, and
    passed every time on a freshly started one - see git history around this test for
    the investigation. Fixed by moving hashing into TranspositionTable itself:
    store()/probe()/bestMove() now take a Board directly and hash it via the table's
    own hashOf() (stone layout + blackCaptured + whiteCaptured + isBlackToPlay), so
    there's exactly one place that computes a TT key and no call site can key a lookup
    off only part of what makes two positions the same.

    Poisons the TT the way this bug was actually found: run a batch of unrelated
    searches first (each one stores nodes reached mid-search, at whatever capture
    counts/turn that search happened to produce, all keyed only by stone layout
    pre-fix), then immediately re-run the "take a free capture" scenario and check it
    still finds 174. A single hand-crafted collision isn't reliable to construct (the
    corrupting entry has to land on a node the real search actually revisits, one ply
    into some candidate line - not the freshly-loaded root, which is never itself
    stored by /analyze); running a representative spread of positions first reproduces
    it the way it was actually observed. Uses a fixed, shallow depth regardless of
    --depth: that's what reliably collides (it's shallow enough that both the priming
    searches and the real one keep revisiting the same handful of near-root nodes -
    deeper searches spread across more distinct positions and the corrupting entry is
    less likely to land somewhere the real search actually probes), and it's also
    exactly the depth this engine ships with by default (AI::maxDepth's default),
    which is what made this bug bite in real, default-config games.
    """
    depth = 3
    grid = {171: WHITE, 172: BLACK, 173: BLACK}

    for t in TESTS:
        analyze(host, port, t["grid"], t["black_to_play"], t["ai_color"], depth)

    data = analyze(host, port, grid, black_to_play=False, ai_color=WHITE, depth=depth)
    move = data["moveHistory"][-1] if data["moveHistory"] else None

    ok = move == 174
    print(f"[{'PASS' if ok else 'FAIL'}] capture-count/turn hash collision regression "
          f"(TT key must include captures+turn, not just stone layout) "
          f"-> played {move}, expected 174")
    return ok


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--host", default="localhost")
    parser.add_argument("--port", type=int, default=9012)
    parser.add_argument("--depth", type=int, default=8, help="searchDepth to use for every test")
    args = parser.parse_args()

    passed = 0
    total = len(TESTS) + 6
    for t in TESTS:
        try:
            data = analyze(args.host, args.port, t["grid"], t["black_to_play"], t["ai_color"], args.depth)
            move = data["moveHistory"][-1] if data["moveHistory"] else None
        except Exception as e:
            print(f"[ERROR] {t['name']}: {e}")
            continue
        ok = move in t["expected"]
        passed += ok
        status = "PASS" if ok else "FAIL"
        print(f"[{status}] {t['name']} -> played {move}, expected one of {sorted(t['expected'])}")

    try:
        passed += check_tt_pollution_regression(args.host, args.port, args.depth)
    except Exception as e:
        print(f"[ERROR] TT staleness regression: {e}")

    try:
        passed += check_self_capture_regression(args.host, args.port, args.depth)
    except Exception as e:
        print(f"[ERROR] self-capture regression: {e}")

    try:
        passed += check_defend_vulnerable_pair_regression(args.host, args.port, args.depth)
    except Exception as e:
        print(f"[ERROR] defend vulnerable pair regression: {e}")

    try:
        passed += check_minmax_board_state_regression(args.host, args.port, args.depth)
    except Exception as e:
        print(f"[ERROR] MINMAX board-state regression: {e}")

    try:
        passed += check_fork_no_false_block_regression(args.host, args.port, args.depth)
    except Exception as e:
        print(f"[ERROR] fork capture-rescue regression: {e}")

    try:
        passed += check_capture_count_hash_collision_regression(args.host, args.port, args.depth)
    except Exception as e:
        print(f"[ERROR] capture-count/turn hash collision regression: {e}")

    print(f"\n{passed}/{total} passed")
    sys.exit(0 if passed == total else 1)


if __name__ == "__main__":
    main()
