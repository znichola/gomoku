#!/usr/bin/env python3
"""
Rules-compliance tests for the Gomoku engine, checked directly against the subject
(en.subject.pdf): captures (appendix VI.1), double-three (appendix VI.2 + the general
guidelines), the "5 or more" win condition, the 10-capture win, and Endgame Capture.

Unlike test/tactics.py (which asks "does the AI make a good choice?" via /analyze on a
throwaway scratch GameState), this suite asks "does the engine enforce the rule?" - it
loads a position directly into the LIVE GameState via /debug-action?action=load-game-state
and then plays one exact move via /move to inspect the resulting board state. That means,
unlike tactics.py, THIS SCRIPT IS NOT SAFE to run against a server with a real game in
progress - point it at a disposable/isolated instance only.

Usage:
    python3 test/rules.py [--host localhost] [--port 9099]
"""
import argparse
import json
import sys
import urllib.request

WIDTH = 19
BLACK, WHITE = 1, 2


def load_game_state(host, port, grid_overrides, black_to_play, black_captured=0, white_captured=0):
    grid = [0] * (WIDTH * WIDTH)
    for idx, val in grid_overrides.items():
        grid[idx] = val
    params = (
        "action=load-game-state&"
        "board_grid=" + ",".join(map(str, grid)) +
        f"&board_width={WIDTH}"
        f"&board_blackCaptured={black_captured}&board_whiteCaptured={white_captured}"
        f"&board_isBlackToPlay={str(black_to_play).lower()}&moveHistory="
    )
    url = f"http://{host}:{port}/debug-action?{params}"
    with urllib.request.urlopen(url, timeout=15) as r:
        return json.load(r)


def play_move(host, port, move_id):
    url = f"http://{host}:{port}/move?id={move_id}"
    with urllib.request.urlopen(url, timeout=15) as r:
        return json.load(r)


def setup_and_play(host, port, grid_overrides, black_to_play, move_id,
                    black_captured=0, white_captured=0):
    """Load a position into the live game, then play exactly one move. Returns the
    resulting board dict (blackCaptured/whiteCaptured/winner/grid/isBlackToPlay)."""
    load_game_state(host, port, grid_overrides, black_to_play, black_captured, white_captured)
    result = play_move(host, port, move_id)
    return result["board"]


# Row 9 (the middle row) starts at id 171 (9*19), so ids 171..189 give a straight line of
# cells without worrying about wrapping to the next/previous row - handy for building
# horizontal patterns. id = row * 19 + col.
CHECKS = []


def check(name):
    def register(fn):
        CHECKS.append((name, fn))
        return fn
    return register


@check("capture removes exactly a pair (subject VI.1)")
def _(host, port):
    # BLACK-WHITE-WHITE, black completes the flank -> the white pair is removed.
    b = setup_and_play(host, port, {171: BLACK, 172: WHITE, 173: WHITE}, True, 174)
    ok = b["blackCaptured"] == 2 and b["grid"][172] == 0 and b["grid"][173] == 0
    return ok, f"blackCaptured={b['blackCaptured']} (want 2), grid[172,173]={b['grid'][172]},{b['grid'][173]} (want 0,0)"


@check("cannot capture a single stone, only pairs (subject VI.1)")
def _(host, port):
    # BLACK-WHITE-EMPTY-<black plays here>: only one enemy stone in between, not a pair.
    b = setup_and_play(host, port, {171: BLACK, 172: WHITE}, True, 174)
    ok = b["blackCaptured"] == 0 and b["grid"][172] == WHITE
    return ok, f"blackCaptured={b['blackCaptured']} (want 0), grid[172]={b['grid'][172]} (want {WHITE})"


@check("cannot capture 3 stones in a row (subject VI.1: 'not more than 2 in a row')")
def _(host, port):
    b = setup_and_play(host, port, {171: BLACK, 172: WHITE, 173: WHITE, 174: WHITE}, True, 175)
    ok = b["blackCaptured"] == 0 and all(b["grid"][i] == WHITE for i in (172, 173, 174))
    return ok, f"blackCaptured={b['blackCaptured']} (want 0), grid[172..174]={[b['grid'][i] for i in (172,173,174)]}"


@check("cannot move into a capture (subject VI.1: 'one can not move into a capture')")
def _(host, port):
    # WHITE-BLACK-<black plays here>-WHITE: completing the pair from the inside, as the
    # to-be-captured color, must not trigger a self-capture (only the flanking color's own
    # move can trigger a capture).
    b = setup_and_play(host, port, {171: WHITE, 172: BLACK, 174: WHITE}, True, 173)
    ok = (b["blackCaptured"] == 0 and b["whiteCaptured"] == 0 and
          b["grid"][171] == WHITE and b["grid"][172] == BLACK and
          b["grid"][173] == BLACK and b["grid"][174] == WHITE)
    return ok, f"captured black={b['blackCaptured']} white={b['whiteCaptured']}, grid={[b['grid'][i] for i in (171,172,173,174)]}"


@check("plain double-three is forbidden (subject: 'No double-threes')")
def _(host, port):
    # Horizontal pair (178,179) and vertical pair (161,142) both aimed at 180: playing
    # there completes two simultaneous open threes.
    grid = {178: BLACK, 179: BLACK, 161: BLACK, 142: BLACK}
    b = setup_and_play(host, port, grid, True, 180)
    ok = b["grid"][180] == 0  # move must have been rejected, cell still empty
    return ok, f"grid[180]={b['grid'][180]} (want 0 = move rejected)"


@check("a single free-three is legal (only the DOUBLE is forbidden)")
def _(host, port):
    grid = {178: BLACK, 179: BLACK}
    b = setup_and_play(host, port, grid, True, 180)
    ok = b["grid"][180] == BLACK
    return ok, f"grid[180]={b['grid'][180]} (want {BLACK} = move accepted)"


@check("double-three IS allowed when the move also captures (subject VI.2 note)")
def _(host, port):
    # Same double-three setup as above, plus a diagonal capture opportunity through 180.
    grid = {178: BLACK, 179: BLACK, 161: BLACK, 142: BLACK, 200: WHITE, 220: WHITE, 240: BLACK}
    b = setup_and_play(host, port, grid, True, 180)
    ok = b["grid"][180] == BLACK and b["blackCaptured"] == 2
    return ok, f"grid[180]={b['grid'][180]} (want {BLACK}), blackCaptured={b['blackCaptured']} (want 2)"


@check("five or more in a row wins, overline included (subject: '5 or more is okay')")
def _(host, port):
    b = setup_and_play(host, port, {171: BLACK, 172: BLACK, 173: BLACK, 174: BLACK}, True, 175)
    ok1 = b["winner"] == BLACK
    b2 = setup_and_play(host, port, {171: BLACK, 172: BLACK, 173: BLACK, 174: BLACK, 175: BLACK}, True, 176)
    ok2 = b2["winner"] == BLACK  # six in a row (overline) must still count as a win
    return ok1 and ok2, f"five: winner={b['winner']} (want {BLACK}), six: winner={b2['winner']} (want {BLACK})"


@check("capturing 10 stones wins outright (subject: base Capture rule)")
def _(host, port):
    # Black already has 8 captured (4 pairs); one more pair brings it to 10 = win by capture.
    b = setup_and_play(host, port, {171: BLACK, 172: WHITE, 173: WHITE}, True, 174, black_captured=8)
    ok = b["blackCaptured"] == 10 and b["winner"] == BLACK
    return ok, f"blackCaptured={b['blackCaptured']} (want 10), winner={b['winner']} (want {BLACK})"


@check("Endgame Capture: a breakable five doesn't win yet (subject Endgame Capture, point 1)")
def _(host, port):
    # Five in a row (100-104), but 102 also pairs with 121, flanked by white 83 (one flank
    # already placed, the other - 140 - still empty): one legal move away from black losing
    # a stone out of the five, so the alignment must not count as a win yet.
    grid = {100: BLACK, 101: BLACK, 102: BLACK, 103: BLACK, 104: BLACK, 121: BLACK, 83: WHITE}
    d = load_game_state(host, port, grid, black_to_play=False)
    ok = d["board"]["winner"] == 0
    return ok, f"winner={d['board']['winner']} (want 0 = not a win yet)"


@check("Endgame Capture: an unbreakable five wins normally (control for the check above)")
def _(host, port):
    grid = {100: BLACK, 101: BLACK, 102: BLACK, 103: BLACK, 104: BLACK}
    d = load_game_state(host, port, grid, black_to_play=False)
    ok = d["board"]["winner"] == BLACK
    return ok, f"winner={d['board']['winner']} (want {BLACK})"


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--host", default="localhost")
    parser.add_argument("--port", type=int, default=9099)
    args = parser.parse_args()

    passed = 0
    for name, fn in CHECKS:
        try:
            ok, detail = fn(args.host, args.port)
        except Exception as e:
            print(f"[ERROR] {name}: {e}")
            continue
        passed += ok
        print(f"[{'PASS' if ok else 'FAIL'}] {name} -> {detail}")

    print(f"\n{passed}/{len(CHECKS)} passed")
    sys.exit(0 if passed == len(CHECKS) else 1)


if __name__ == "__main__":
    main()
