#pragma once

#include "Grid.hpp"
#include "Board.hpp"
#include <vector>
#include <cstdint>

/*
    This defines what type of score is saved
    EXACT score can just be used
    LOWER is an assured lowerbound, this can inform alpha
    UPPER is an assured upperbound, this can infrom beta
*/
enum class Bound : uint8_t { EXACT, LOWER, UPPER };

struct TTEntry {
    uint64_t hash = 0;
    float    score = 0.0f;
    int16_t  depth = 0;
    int      move = -1; // best move index, -1 if none, TT move heuristic
    Bound    bound = Bound::EXACT;
    uint8_t  age = 0; // Each move in a game increments age, so we can prefer later moves
};

class TranspositionTable {
public:
    static constexpr size_t SIZE = 1 << 23; // must be power of 2

    TranspositionTable();

    void newSearch();
    void store(uint64_t hash, float score, int16_t depth, int move, Bound bound);
    const TTEntry* probe(uint64_t hash) const;
    int bestMove(uint64_t hash) const;
    void clear();

private:
    std::vector<TTEntry> table;
    size_t mask;
    uint8_t age;
};