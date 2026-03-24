#include <sstream>
#include <iostream>

#include "GameState.hpp"

std::string GameState::serialize() const {
    std::ostringstream out;

    out << "{\n";
    out << "  \"isHumanGame\": " << (isHumanGame ? "true" : "false") << ",\n";
    out << "  \"movesPlayed\": " << moveHistory.size() << ",\n";
    out << "  \"board\": " << board.serialize() << "\n";
    out << "}";

    return out.str();
}

bool GameState::playMove(unsigned id) {
    std::cout << "Playing move: " << id << "\n";
    moveHistory.push_back(id);
    return board.playMove(id);
}