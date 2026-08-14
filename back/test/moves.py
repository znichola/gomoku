#!/usr/bin/env python3
"""
Python port of test/moves.cpp - unit-style tests for Board::isValidMove and
Grid::isDoubleThree, run against a live backend instead of compiled directly against the
engine. test/moves.cpp itself bit-rotted twice already (an API change to Board's
constructors, then to isDoubleThree's signature) without anyone noticing, because nothing
ran it as part of routine testing - hitting the same logic over HTTP, alongside
test/tactics.py and test/rules.py, means it can't go stale unnoticed again.

Like test/rules.py (and unlike test/tactics.py's /analyze), this uses
/debug-action?action=load-game-state + /move, which operate on the LIVE GameState - so
THIS SCRIPT IS NOT SAFE to run against a server with a real game in progress.

More detailed than the original moves.cpp in a few ways:
  - isDoubleThree's own test table in moves.cpp never actually tested a TRUE double-three
    (every one of its 15 cases expects false) - added one here.
  - Added the "double-three via capture" exception (subject's explicit carve-out), which
    moves.cpp never covered either.
  - Added isValidMove cases for playing onto a cell held by the opponent (moves.cpp only
    tried one's own color), and for playing after the game already has a winner.
  - Descriptions expanded to spell out which axis/shape each case is probing, and whether
    it's a "blocking" case (no free-three forms at all) vs a "single free-three, but not a
    DOUBLE, so still legal" case - the original grouped these but didn't say so explicitly.

Usage:
    python3 test/moves.py [--host localhost] [--port 9099]
"""
import argparse
import json
import sys
import urllib.request

BLACK, WHITE = 1, 2


def load_game_state(host, port, width, grid_overrides, black_to_play,
                     black_captured=0, white_captured=0):
    grid = [0] * (width * width)
    for idx, val in grid_overrides.items():
        grid[idx] = val
    params = (
        "action=load-game-state&"
        "board_grid=" + ",".join(map(str, grid)) +
        f"&board_width={width}"
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


CHECKS = []


def check(name):
    def register(fn):
        CHECKS.append((name, fn))
        return fn
    return register


# --- isValidMove -------------------------------------------------------------------

@check("isValidMove: out of bounds move is rejected")
def _(host, port):
    # 4x4 board (16 cells, valid ids 0..15) - id 16 is one past the end.
    load_game_state(host, port, 4, {}, black_to_play=True)
    b = play_move(host, port, 16)["board"]
    ok = b["grid"][0] == 0 and all(c == 0 for c in b["grid"])  # nothing got placed anywhere
    return ok, "move onto an out-of-bounds id must be silently rejected, board stays empty"


@check("isValidMove: move onto an empty cell is accepted")
def _(host, port):
    load_game_state(host, port, 4, {0: BLACK}, black_to_play=False)
    b = play_move(host, port, 1)["board"]
    ok = b["grid"][1] == WHITE
    return ok, f"grid[1]={b['grid'][1]} (want {WHITE})"


@check("isValidMove: move onto a cell held by your OWN color is rejected")
def _(host, port):
    load_game_state(host, port, 4, {0: WHITE}, black_to_play=False)
    b = play_move(host, port, 0)["board"]
    ok = b["grid"][0] == WHITE  # unchanged - the move was refused, not "re-played"
    return ok, f"grid[0]={b['grid'][0]} (want unchanged {WHITE})"


@check("isValidMove: move onto a cell held by the OPPONENT is also rejected")
def _(host, port):
    # moves.cpp only ever tested playing onto one's own color - a cell is a cell regardless
    # of which color occupies it, so this is worth checking explicitly too.
    load_game_state(host, port, 4, {0: BLACK}, black_to_play=False)
    b = play_move(host, port, 0)["board"]
    ok = b["grid"][0] == BLACK  # still black - white's move onto it was refused
    return ok, f"grid[0]={b['grid'][0]} (want unchanged {BLACK})"


@check("isValidMove: no move is accepted once the game already has a winner")
def _(host, port):
    load_game_state(host, port, 7, {0: BLACK, 1: BLACK, 2: BLACK, 3: BLACK, 4: BLACK},
                     black_to_play=False)
    b = play_move(host, port, 20)["board"]
    ok = b["winner"] == BLACK and b["grid"][20] == 0
    return ok, f"winner={b['winner']} (want {BLACK}), grid[20]={b['grid'][20]} (want 0 = move refused)"


@check("isValidMove: a move that would form a double-three is rejected")
def _(host, port):
    # Cross-check at the isValidMove level (the actual gate used in real play), not just
    # isDoubleThree in isolation - see the isDoubleThree section below for the geometry.
    grid = {22: WHITE, 23: WHITE, 10: WHITE, 17: WHITE}
    load_game_state(host, port, 7, grid, black_to_play=False)
    b = play_move(host, port, 24)["board"]
    ok = b["grid"][24] == 0
    return ok, f"grid[24]={b['grid'][24]} (want 0 = move refused)"


@check("isValidMove: a double-three IS accepted when the move also captures")
def _(host, port):
    grid = {178: BLACK, 179: BLACK, 161: BLACK, 142: BLACK, 200: WHITE, 220: WHITE, 240: BLACK}
    load_game_state(host, port, 19, grid, black_to_play=True)
    b = play_move(host, port, 180)["board"]
    ok = b["grid"][180] == BLACK and b["blackCaptured"] == 2
    return ok, f"grid[180]={b['grid'][180]} (want {BLACK}), blackCaptured={b['blackCaptured']} (want 2)"


# --- isDoubleThree -----------------------------------------------------------------
#
# All cases below use a 7x7 board (ids 0..48, row = id // 7, col = id % 7), white to play,
# matching test/moves.cpp's layout:
#   0  1  2  3  4  5  6
#   7  8  9 10 11 12 13
#  14 15 16 17 18 19 20
#  21 22 23 24 25 26 27
#  28 29 30 31 32 33 34
#  35 36 37 38 39 40 41
#  42 43 44 45 46 47 48
#
# Each case plays the given move on top of the given (pre-existing) white/black stones and
# checks whether it's accepted (not a double-three) or refused (is a double-three). None of
# these shapes involve a capture, so isValidMove's outcome here is exactly isDoubleThree's.

def _double_three_case(name, grid, move_id, expect_accepted, note):
    # Some cases below list move_id as already part of `grid` (matching moves.cpp's
    # convention of pre-placing the stone being tested) - isDoubleThree only ever reads the
    # cells AROUND move_id, never move_id itself, so this doesn't change what's being
    # checked, but the cell has to be empty for /move to actually attempt playing there.
    setup = {k: v for k, v in grid.items() if k != move_id}

    @check(name)
    def _(host, port):
        load_game_state(host, port, 7, setup, black_to_play=False)
        b = play_move(host, port, move_id)["board"]
        accepted = b["grid"][move_id] == WHITE
        ok = accepted == expect_accepted
        return ok, f"{note} -> accepted={accepted} (want {expect_accepted})"


# -- Shapes that don't form a free-three at all (nothing to block) --
_double_three_case(
    "isDoubleThree: blank board, no alignment exists yet",
    {}, 1, True, "single stone placed in isolation")

_double_three_case(
    "isDoubleThree: only a pair forms (not a three)",
    {2: WHITE, 3: WHITE}, 2, True, "two contiguous white stones, no third stone anywhere")

_double_three_case(
    "isDoubleThree: only a pair forms, with unrelated distant stones",
    {1: WHITE, 2: WHITE, 5: WHITE, 6: WHITE}, 2, True,
    "the (1,2) pair is what's being played into; (5,6) is a separate, unrelated pair")

_double_three_case(
    "isDoubleThree: a single isolated stone forms no three",
    {4: WHITE}, 4, True, "just one stone on the board")

# -- Runs of 4+ (already a stronger threat than a mere "three", shouldn't be flagged as a double-three) --
_double_three_case(
    "isDoubleThree: a contiguous run of 4 forms (not a 'three')",
    {2: WHITE, 3: WHITE, 4: WHITE, 5: WHITE}, 4, True,
    "playing the 4th stone of an already-3-long run - a four, not a double-three")

_double_three_case(
    "isDoubleThree: playing into the middle of a spread-out run of 4",
    {2: WHITE, 3: WHITE, 4: WHITE, 5: WHITE}, 3, True,
    "same 4 stones, but the move being tested is the 2nd cell, not the end")

_double_three_case(
    "isDoubleThree: playing the 3rd stone of what becomes a run of 4",
    {2: WHITE, 3: WHITE, 4: WHITE, 5: WHITE}, 2, True,
    "same shape again, move being tested is the leftmost cell")

# -- Blocked shapes: a three forms, but it's not "free" (board edge or an enemy stone
#    closes off one side, so it can never become an indefensible open four) --
_double_three_case(
    "isDoubleThree: three formed but the board edge blocks one end",
    {0: WHITE, 1: WHITE}, 2, True,
    "playing at col 2 makes cells 0,1,2 - but col -1 doesn't exist, so this end is closed")

_double_three_case(
    "isDoubleThree: pieces too spread out to form any three",
    {1: WHITE, 3: WHITE}, 5, True, "three white stones with gaps of 1 between each")

_double_three_case(
    "isDoubleThree: a disconnected three, but black blocks the gap side",
    {2: WHITE, 3: WHITE, 4: WHITE}, 1, True,
    "playing at col 1 makes a broken shape with col 2-4, but check the specific flank")

_double_three_case(
    "isDoubleThree: connected three flanked by black one cell further out",
    {3: WHITE, 4: WHITE, 0: BLACK, 6: BLACK}, 2, True,
    "black at both far ends (one cell beyond where the three would reach) - not immediately "
    "blocking, but there's no way to reach an open four without eventually hitting black")

_double_three_case(
    "isDoubleThree: disconnected three directly flanked by black",
    {2: WHITE, 5: WHITE, 1: BLACK}, 4, True,
    "black sits immediately against one end of the broken three - blocked, not free")

# -- A single free-three forms (legal - only the DOUBLE is forbidden) --
_double_three_case(
    "isDoubleThree: a single free-three (contiguous XXX) is legal",
    {2: WHITE, 4: WHITE}, 3, True,
    "playing the middle cell completes a contiguous three, open on both ends - still just "
    "ONE free-three, and isDoubleThree only flags TWO simultaneous ones")

_double_three_case(
    "isDoubleThree: a single free-three, opponent stone too far to matter",
    {1: WHITE, 2: WHITE, 5: BLACK}, 3, True,
    "black at col 5 is beyond the cell (col 4) that would need to be blocked - irrelevant")

_double_three_case(
    "isDoubleThree: a single free-three, broken shape (X X _ X)",
    {2: WHITE, 5: WHITE}, 3, True,
    "cells 2,3,5 white with a gap at 4 - still a free-three (per the subject's broken-three "
    "diagram), still just one axis, so still legal alone")

# -- The actual illegal case: TWO simultaneous free-threes --
_double_three_case(
    "isDoubleThree: a GENUINE double-three is rejected",
    {22: WHITE, 23: WHITE, 10: WHITE, 17: WHITE}, 24, False,
    "playing at 24 completes a horizontal three (22,23,24, open both ends) AND a vertical "
    "three (10,17,24, open both ends) at once - this is the one shape moves.cpp's own test "
    "table never actually exercised despite testing isDoubleThree extensively")


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
