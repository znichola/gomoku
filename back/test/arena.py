#!/usr/bin/env python3
"""
Play two compiled gomoku server binaries against each other and report the results -
for comparing an old savepoint binary against the one you're currently developing.

Each binary is its own full server (its own port, its own AI, its own live GameState -
see main.cpp: PORT is baked in at compile time via `make PORT=9013`). This script starts
both as subprocesses, gives each the SAME engine config (from a JSON config file shaped
just like the JSON /api/set-config returns - searchDepth, aiTimeBudgetMs, searchFunction,
moveFunction), then referees: it asks one side's binary to compute a move via
/api/ai-move, plays that exact move on the other binary's board via /api/move?id=, asks
that one to move, and so on - each binary only ever computes moves for its own color, the
two boards are kept in lockstep by mirroring every move across.

The OLD binary always plays BLACK (first-move advantage) and the NEW binary always plays
WHITE, for every match in the run - not alternated. That's deliberate: it's a harder bar
for the new binary to clear, so a >50% win rate for "new" is a real, not lucky, signal.

Usage:
    python3 test/arena.py 100 ./bin_old ./bin_new
    python3 test/arena.py 100 ./bin_old ./bin_new --config arena_config.example.json
    python3 test/arena.py 20 ./bin_old ./bin_new --port-old 9013 --port-new 9014 --out results.json

The two binaries must already be built for two different ports, e.g.:
    make PORT=9013 NAME=bin_old
    make PORT=9014 NAME=bin_new
"""
import argparse
import json
import os
import subprocess
import sys
import time
import urllib.error
import urllib.parse
import urllib.request

WINNER_NAME = {0: None, 1: "old", 2: "new"}  # Cell::EMPTY/BLACK/WHITE - old is always black

# isAIGame (and board colors generally) are the raw Cell enum values as strings, not
# names - parseCell() in Utils.cpp only accepts "0"/"1"/"2" and throws (-> HTTP 500) on
# anything else, unlike e.g. searchFunction which does take names.
BLACK, WHITE = "1", "2"


def http_get(url, timeout):
    with urllib.request.urlopen(url, timeout=timeout) as r:
        return json.load(r)


def api(host, port, path, params, timeout):
    qs = urllib.parse.urlencode(params)
    url = f"http://{host}:{port}{path}"
    if qs:
        url += f"?{qs}"
    return http_get(url, timeout)


class Engine:
    """One running server subprocess, addressed by host:port."""

    def __init__(self, label, binary, host, port, log_path):
        self.label = label
        # subprocess.Popen (like exec()) won't search cwd for a bare filename the way an
        # interactive shell resolves "./name" - only PATH. Resolve a bare "gomoku_bench"
        # sitting in the current directory to an explicit path so it doesn't need "./".
        if os.sep not in binary and not os.path.isabs(binary) and os.path.isfile(binary):
            binary = os.path.join(".", binary)
        if not os.path.isfile(binary):
            raise RuntimeError(f"{label} binary not found: {binary}")
        if not os.access(binary, os.X_OK):
            raise RuntimeError(f"{label} binary not executable: {binary}")
        self.binary = binary
        self.host = host
        self.port = port
        self.log_fh = open(log_path, "w")
        self.proc = subprocess.Popen([binary], stdout=self.log_fh, stderr=subprocess.STDOUT)

    def wait_ready(self, timeout):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            if self.proc.poll() is not None:
                raise RuntimeError(
                    f"{self.label} ({self.binary}) exited during startup "
                    f"(code {self.proc.returncode}) - see {self.log_fh.name}")
            try:
                api(self.host, self.port, "/api/gameState", {}, timeout=1)
                return
            except (urllib.error.URLError, ConnectionError, TimeoutError):
                time.sleep(0.2)
        raise RuntimeError(
            f"{self.label} ({self.binary}) never answered on {self.host}:{self.port} "
            f"within {timeout}s - see {self.log_fh.name}")

    def set_config(self, params, timeout):
        return api(self.host, self.port, "/api/set-config", params, timeout)

    def reset(self, timeout):
        return api(self.host, self.port, "/api/reset", {}, timeout)

    def ai_move(self, timeout):
        return api(self.host, self.port, "/api/ai-move", {}, timeout)

    def play_move(self, move_id, timeout):
        return api(self.host, self.port, "/api/move", {"id": move_id}, timeout)

    def stop(self):
        if self.proc.poll() is None:
            self.proc.terminate()
            try:
                self.proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.proc.kill()
                self.proc.wait()
        self.log_fh.close()


class MatchError(Exception):
    pass


def play_match(old, new, timeout, max_moves):
    """Old plays black (moves first), new plays white. Returns a result dict."""
    started = time.monotonic()

    old_state = old.reset(timeout)
    new.reset(timeout)

    last_move = old_state["moveHistory"][-1] if old_state["moveHistory"] else None
    if last_move is None:
        raise MatchError("old binary did not auto-play move 1 - check isAIGame=black got applied")

    state = old_state
    mover = new  # who computes the *next* move, after the pending one below is mirrored onto them
    move_count = 1

    while state["board"]["winner"] == 0 and move_count < max_moves:
        mirrored = mover.play_move(last_move, timeout)
        if len(mirrored["moveHistory"]) != move_count:
            raise MatchError(
                f"move {last_move} was rejected when mirrored onto {mover.label} "
                f"(moveHistory length {len(mirrored['moveHistory'])}, expected {move_count})")

        if mirrored["board"]["winner"] != 0:
            state = mirrored
            break

        played = mover.ai_move(timeout)
        last_move = played["moveHistory"][-1]
        move_count += 1
        state = played
        mover = old if mover is new else new

    winner_cell = state["board"]["winner"]
    result = {
        "winner": WINNER_NAME.get(winner_cell, "draw" if move_count >= max_moves else "unknown"),
        "moves": move_count,
        "moveHistory": state["moveHistory"],
        "winByCaptures": state["board"]["winByCaptures"],
        "winByAlignment": state["board"]["winByAlignment"],
        "blackCaptured": state["board"]["blackCaptured"],
        "whiteCaptured": state["board"]["whiteCaptured"],
        "durationSec": round(time.monotonic() - started, 3),
    }
    if result["winner"] is None:
        result["winner"] = "draw"
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("matches", type=int, help="number of matches to play")
    parser.add_argument("bin_old", help="path to the old/baseline binary (always plays black)")
    parser.add_argument("bin_new", help="path to the new/candidate binary (always plays white)")
    parser.add_argument("--config", default=None,
                         help="JSON file shaped like the /api/set-config response "
                              "(searchDepth, aiTimeBudgetMs, searchFunction, moveFunction), "
                              "applied identically to both binaries. See arena_config.example.json")
    parser.add_argument("--host", default="localhost")
    parser.add_argument("--port-old", type=int, default=9013)
    parser.add_argument("--port-new", type=int, default=9014)
    parser.add_argument("--out", default="arena_results.json")
    parser.add_argument("--move-timeout", type=float, default=60,
                         help="seconds to wait for a single /api/ai-move response")
    parser.add_argument("--startup-timeout", type=float, default=10)
    parser.add_argument("--max-moves", type=int, default=361,
                         help="safety cap on moves per match before it's called a draw")
    args = parser.parse_args()

    config = {}
    if args.config:
        with open(args.config) as f:
            config = json.load(f)

    old = None
    new = None
    results = []
    try:
        print(f"starting old={args.bin_old} on :{args.port_old}, "
              f"new={args.bin_new} on :{args.port_new} ...", flush=True)
        old = Engine("old", args.bin_old, args.host, args.port_old, "/tmp/arena_old.log")
        new = Engine("new", args.bin_new, args.host, args.port_new, "/tmp/arena_new.log")

        old.wait_ready(args.startup_timeout)
        new.wait_ready(args.startup_timeout)

        old.set_config({**config, "isAIGame": BLACK}, args.move_timeout)
        new.set_config({**config, "isAIGame": WHITE}, args.move_timeout)
        print("both servers ready, playing...", flush=True)

        for i in range(1, args.matches + 1):
            try:
                result = play_match(old, new, args.move_timeout, args.max_moves)
            except (MatchError, urllib.error.URLError, TimeoutError) as e:
                result = {"winner": "error", "error": str(e)}
            result["match"] = i
            results.append(result)
            print(f"[{i}/{args.matches}] winner={result['winner']:<6} "
                  f"moves={result.get('moves', '-')} "
                  f"{'(captures)' if result.get('winByCaptures') else ''}"
                  f"{'(alignment)' if result.get('winByAlignment') else ''}",
                  flush=True)
    finally:
        if old is not None:
            old.stop()
        if new is not None:
            new.stop()

    old_wins = sum(1 for r in results if r["winner"] == "old")
    new_wins = sum(1 for r in results if r["winner"] == "new")
    draws = sum(1 for r in results if r["winner"] == "draw")
    errors = sum(1 for r in results if r["winner"] == "error")
    n = len(results)

    print("\n--- summary ---")
    print(f"matches played : {n}")
    print(f"old (black) wins: {old_wins} ({old_wins / n:.1%})" if n else "old wins: 0")
    print(f"new (white) wins: {new_wins} ({new_wins / n:.1%})" if n else "new wins: 0")
    print(f"draws           : {draws}")
    print(f"errors          : {errors}")

    summary = {
        "binOld": args.bin_old,
        "binNew": args.bin_new,
        "config": config,
        "matches": n,
        "oldWins": old_wins,
        "newWins": new_wins,
        "draws": draws,
        "errors": errors,
        "results": results,
    }
    with open(args.out, "w") as f:
        json.dump(summary, f, indent=2)
    print(f"\nfull match history written to {args.out}")

    sys.exit(0)


if __name__ == "__main__":
    main()
