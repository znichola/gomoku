#include <sstream>

#include "Board.hpp"

std::string Board::serialize() const {
    std::ostringstream out;

    out << "{\n";
    out << "  \"blackCaptured\": " << blackCaptured << ",\n";
    out << "  \"whiteCaptured\": " << whiteCaptured << ",\n";
    out << "  \"isBlackToPlay\": " << (isBlackToPlay ? "true" : "false") << ",\n";
    out << "  \"grid\": [";
    for (size_t i = 0; i < grid.size(); ++i) {
        out << static_cast<int>(grid[i]);
        if (i + 1 < grid.size())
            out << ",";
    }
    out << "]\n";
    out << "}";

    return out.str();
}
