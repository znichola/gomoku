#include "AI.hpp"
#include "MessageQueue.hpp"

// https://en.wikipedia.org/wiki/Minimax#Pseudocode
unsigned AI::play(const Board &board, unsigned lastMove) {
    // auto h = heuristic(board, lastMove);
    // if (h.size() > 0) {
    //     MQ << "AI failed to find a valid move :(";
    // }
    (void)board;
    return 360 - lastMove;
}
/*
std::vector<std::pair<unsigned, double>>AI::heuristic(const Board &board) {
    return {{}};
}

unsigned AI::minMax(const Board &board, int depth, bool maximizingPlayer) {
    if (depth == 0 || board.isWin()) {
        return heuristic(board);
    } 
}
*/
