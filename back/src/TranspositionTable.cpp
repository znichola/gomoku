#include <algorithm>
#include <cassert>

#include "TranspositionTable.hpp"

TranspositionTable::TranspositionTable(): table(SIZE), mask(SIZE - 1), age(0) {}

void TranspositionTable::newSearch() {
    age++;
}

void TranspositionTable::store(uint64_t hash, float score, int16_t depth, int move, Bound bound) {
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

const TTEntry* TranspositionTable::probe(uint64_t hash) const {
    const TTEntry& e = table[hash & mask];
    return (e.hash == hash) ? &e : nullptr;
}

int TranspositionTable::bestMove(uint64_t hash) const {
    const TTEntry* e = probe(hash);
    return e ? e->move : Board::FIRSTMOVE;
}

void TranspositionTable::clear() {
    std::fill(table.begin(), table.end(), TTEntry{});
}