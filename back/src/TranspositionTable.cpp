#include <algorithm>
#include <cassert>

#include "TranspositionTable.hpp"

TranspositionTable::TranspositionTable(): table(SIZE), mask(SIZE - 1), age(0) {}

void TranspositionTable::newSearch() {
    age++;
}

void TranspositionTable::resetAge() {
    age = 0;
}

uint64_t TranspositionTable::hashOf(const Board &board) {
    uint64_t h = board.grid.getHash();
    h ^= (static_cast<uint64_t>(board.blackCaptured) * 0x9E3779B97F4A7C15ULL);
    h ^= (static_cast<uint64_t>(board.whiteCaptured) * 0xC2B2AE3D27D4EB4FULL);
    if (board.isBlackToPlay) h ^= 0xD1B54A32D192ED03ULL;
    return h;
}

void TranspositionTable::store(const Board &board, float score, int16_t depth, int move, Bound bound) {
    uint64_t hash = hashOf(board);
    TTEntry& e = table[hash & mask];

    // Keep deeper entries unless they're stale
    if (e.hash == hash || e.age != age || e.depth <= depth) {
        e.hash = hash;
        e.score = score;
        e.depth = static_cast<int16_t>(depth);
        e.move = move;
        e.bound = bound;
        e.age = age;
    }
}

const TTEntry* TranspositionTable::probe(const Board &board) const {
    uint64_t hash = hashOf(board);
    const TTEntry& e = table[hash & mask];
    return (e.hash == hash) ? &e : nullptr;
}

int TranspositionTable::bestMove(const Board &board) const {
    const TTEntry* e = probe(board);
    return e ? e->move : Board::FIRSTMOVE;
}

void TranspositionTable::clear() {
    std::fill(table.begin(), table.end(), TTEntry{});
}