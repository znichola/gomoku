#include <sstream>

#include "Board.hpp"

bool Board::playMove(unsigned id) {
    if (!isValidMove(id))
        return false;

    grid[id] = isBlackToPlay ? Cell::BLACK : Cell::WHITE;
    isBlackToPlay = !isBlackToPlay;
    return true;
}

bool Board::isValidMove(unsigned id) const {
    if (id >= grid.size())
        return false;

    if (grid[id] != Cell::EMPTY)
        return false;

    return true;
}

std::string Board::serialize() const {
    std::ostringstream out;

    out << "{\n";
    out << "\"boardSize\": " << boardSize << ",\n";
    out << "\"blackCaptured\": " << blackCaptured << ",\n";
    out << "\"whiteCaptured\": " << whiteCaptured << ",\n";
    out << "\"isBlackToPlay\": " << (isBlackToPlay ? "true" : "false") << ",\n";
    out << "\"grid\": [";
    for (size_t i = 0; i < grid.size(); ++i) {
        out << static_cast<int>(grid[i]);
        if (i + 1 < grid.size())
            out << ",";
    }
    out << "]\n";
    out << "}";

    return out.str();
}
