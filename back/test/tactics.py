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
            black_captured=0, white_captured=0):
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
    url = f"http://{host}:{port}/analyze?{params}"
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


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--host", default="localhost")
    parser.add_argument("--port", type=int, default=9012)
    parser.add_argument("--depth", type=int, default=8, help="searchDepth to use for every test")
    args = parser.parse_args()

    passed = 0
    total = len(TESTS) + 2
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

    print(f"\n{passed}/{total} passed")
    sys.exit(0 if passed == total else 1)


if __name__ == "__main__":
    main()
