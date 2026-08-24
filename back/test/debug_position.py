#!/usr/bin/env python3
"""
Dump the full /analyze output (board, every log message) for a hand-built position.
Useful for stepping through what the AI's search actually saw and scored at each
iterative-deepening depth, when a tactics.py regression test fails and you need more
than pass/fail to figure out why.

Positions are defined as Python dicts below (edit POSITION to try a new one, or import
`analyze`/`print_board` from this file in a throwaway script). id = row * 19 + col.

Usage:
    python3 test/debug_position.py [--host localhost] [--port 9012] [--depth 8]
"""
import argparse
import json
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
    url = f"http://{host}:{port}/api/analyze?{params}"
    with urllib.request.urlopen(url, timeout=30) as r:
        return json.load(r)


def print_board(grid_overrides, focus=()):
    xs = list(grid_overrides.keys()) + list(focus)
    if not xs:
        print("(empty board)")
        return
    cols = [i % WIDTH for i in xs]
    rows = [i // WIDTH for i in xs]
    y0, y1 = max(0, min(rows) - 1), min(WIDTH - 1, max(rows) + 1)
    x0, x1 = max(0, min(cols) - 1), min(WIDTH - 1, max(cols) + 1)
    print("     " + "".join(str(x % 10) for x in range(x0, x1 + 1)))
    for y in range(y0, y1 + 1):
        row = ""
        for x in range(x0, x1 + 1):
            idx = y * WIDTH + x
            if idx in focus:
                row += "*"
            elif grid_overrides.get(idx) == BLACK:
                row += "B"
            elif grid_overrides.get(idx) == WHITE:
                row += "W"
            else:
                row += "."
        print(f"{y:4d} {row}")


# Edit this to whatever position you're currently debugging.
POSITION = {
    "black": [142, 124, 106, 88],
    "white": [160, 144, 162],
    "black_to_play": False,
    "ai_color": WHITE,
    "expect": 70,
    "note": "minimal repro: same diagonal four, plus just 144+162 - this alone made white play 180 (dead center, unrelated to anything)",
}


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--host", default="localhost")
    parser.add_argument("--port", type=int, default=9012)
    parser.add_argument("--depth", type=int, default=8)
    args = parser.parse_args()

    grid = {m: BLACK for m in POSITION["black"]}
    grid.update({m: WHITE for m in POSITION["white"]})

    print(POSITION.get("note", ""))
    print_board(grid, focus={POSITION["expect"]} if "expect" in POSITION else ())
    print()

    d = analyze(args.host, args.port, grid, POSITION["black_to_play"], POSITION["ai_color"], args.depth)
    move = d["moveHistory"][-1] if d["moveHistory"] else None
    expect = POSITION.get("expect")
    print(f"played: {move}" + (f"  (expected {expect}, {'OK' if move == expect else 'MISMATCH'})" if expect else ""))
    print(f"aiMoveMicros: {d.get('aiMoveMicros')}")
    print()
    print("--- full message log ---")
    for m in d.get("messages", []):
        print(m)


if __name__ == "__main__":
    main()
