#include <sstream>
#include <iostream>

#include "GameState.hpp"

std::string GameState::serialize() const {
    std::ostringstream out;

    out << "{\n";
    out << "  \"isHumanGame\": " << (isHumanGame ? "true" : "false") << ",\n";
    out << "  \"movesPlayed\": " << movesPlayed << ",\n";
    out << "  \"board\": " << board.serialize() << "\n";
    out << "}";

    return out.str();
}

bool GameState::doMove(unsigned id) {
    std::cout << "Playing move: " << id << "\n";
    return true;
}