#include <sstream>
#include <iostream>

#include "GameState.hpp"

std::string GameState::serialize() const {
    std::ostringstream out;

    out << "{\n";
    out << "\"isHumanGame\": " << (isHumanGame ? "true" : "false") << ",\n";
    out << "\"moveHistory\": [";
    for (size_t i = 0; i < moveHistory.size(); ++i) {
        out << moveHistory[i];
        if (i + 1 < moveHistory.size())
            out << ",";
    }
    out << "],\n\"board\": " << board.serialize() << "\n";
    out << "}";

    return out.str();
}

bool GameState::playMove(unsigned id) {
    std::cout << "Playing move: " << id << "\n";
    moveHistory.push_back(id);
    return board.playMove(id);
}

void GameState::reset() {
    board = Board(board.grid.boardDimentions);
    moveHistory.clear();
}