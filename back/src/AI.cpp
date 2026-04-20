#include <iostream>

#include "AI.hpp"
#include "MessageQueue.hpp"

// https://en.wikipedia.org/wiki/Minimax#Pseudocode
unsigned AI::play(const Board &board, unsigned lastMove) {
    auto h = heuristic(board);
    if (h.size() > 0) {
        // for (auto [id, hh ] : h) MBQ(id, std::to_string(hh));
    } else {
        MQ << "heuristic is empty :(";
    }
    (void)board;
    const unsigned d = board.grid.boardDimension;
    const long nid = d * d - lastMove;
    long id = nid;
    long i = 0;
    while (board.grid.grid[id] != Cell::EMPTY && i < d * d) {
        id = d * d / 2 - (i / 2) * ((i % 2) ? -1 : 1);
        i++;
    }
    return id;
}

std::vector<std::pair<unsigned, double>>AI::heuristic(const Board &board) {
    std::set<unsigned> cm = getCandidateMoves(board.grid);
    std::vector<std::pair<unsigned, double>> result;
    for (auto id: cm) {
        result.push_back({id, 3});
    }
    std::cout << "FOOOO " << result.size(); 
    return result;
}

/*
unsigned AI::minMax(const Board &board, int depth, bool maximizingPlayer) {
    if (depth == 0 || board.isWin()) {
        return heuristic(board);
    } 
}
*/

std::set<unsigned> AI::getCandidateMoves(const Grid &grid) {
    std::set<unsigned> cm;
    for (size_t id = 0; id < grid.size(); id++) {
        if (grid.grid[id] == Cell::EMPTY) continue;
        Vector2D v = grid.idToVec(id);
        for (const Vector2D &e: EXTREMITIES) {
            Vector2D nv = v + e;
            if (!grid.isInside(nv)) continue;
            unsigned nid = grid.vecToId(nv);
            if (grid[nid] == Cell::EMPTY)
                cm.insert(nid);
        }
    }
    return cm;
}