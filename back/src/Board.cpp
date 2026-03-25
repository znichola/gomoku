#include <sstream>

#include "Board.hpp"
#include "Cell.hpp"

bool Board::playMove(unsigned id) {
    if (!isValidMove(id))
        return false;

    grid[id] = isBlackToPlay ? Cell::BLACK : Cell::WHITE;
    isBlackToPlay = !isBlackToPlay;
    return true;
}

bool Board::isValidMove(unsigned id) {
    if (id >= grid.size())
        return false;

    if (grid[id] != Cell::EMPTY)
        return false;

    // is it part of two three free
    if (isBlackToPlay) grid.setBlack(id); else grid.setWhite(id);

    if (grid.threeFreesPlayedPieceIsPartOf(id) >= 2)
        return false;

    grid.setEmpty(id);

    return true;
}

std::string Board::serialize() const {
    std::ostringstream out;

    out << "{\n";
    out << "\"boardDimentions\": " << grid.boardDimentions << ",\n";
    out << "\"blackCaptured\": " << blackCaptured << ",\n";
    out << "\"whiteCaptured\": " << whiteCaptured << ",\n";
    out << "\"isBlackToPlay\": " << (isBlackToPlay ? "true" : "false") << ",\n";
    out << "\"grid\": " << grid.serialize() << "\n";
    out << "}";

    return out.str();
}
