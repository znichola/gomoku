#include <sstream>
#include <iostream>

#include "Board.hpp"
#include "Cell.hpp"

bool Board::playMove(unsigned id) {
    if (!isValidMove(id))
        return false;

    grid[id] = isBlackToPlay ? Cell::BLACK : Cell::WHITE;

    doCaltures(id);

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

void Board::doCaltures(unsigned id) {
    const Cell myColor = grid[id];
    if (myColor == Cell::EMPTY) return;
    const Cell enemyColor = (myColor == Cell::BLACK ? Cell::WHITE : Cell::BLACK);
    const unsigned d = grid.boardDimentions;
    const std::initializer_list<std::pair<long, long>> extremities = {
        { 1,  0}, // right
        { 1,  1}, // bottom-right
        { 0,  1}, // bottom
        {-1,  1}, // bottom-left
        {-1,  0}, // left
        {-1, -1}, // top-left
        { 0, -1}, // top
        { 1, -1}  // top-right
    };

    for (auto [x, y] : extremities) {
        const long nx = id % d + x * 3;
        const long ny = id / d + y * 3;
        const long e = ny * d + nx;
        std::cout << e << " [" << nx << "; " << ny << "] ";
        if (!(0 <= nx && nx < d && 0 <= ny && ny < d)) continue;
        std::cout << " " << (myColor == Cell::BLACK ? "B" : myColor == Cell::WHITE ? "W" : "E") 
            << "  " << (grid[e] == Cell::BLACK ? "B" : grid[e] == Cell::WHITE ? "W" : "E") << "\n";
        if (grid[e] != myColor) continue;
        const long e1 = (id / d + y * 1) * d + (id % d + x * 1);
        const long e2 = (id / d + y * 2) * d + (id % d + x * 2);
        std::cout << "E1:" << e1 << " E2:" << e2 << "\n";
        if (grid[e1] == enemyColor && grid[e2] == enemyColor) {
            grid.setEmpty(static_cast<unsigned>(e1));
            grid.setEmpty(static_cast<unsigned>(e2));
            addCapture(myColor);
        };
    }
}

void Board::addCapture(Cell color) {
    if (color != Cell::EMPTY) return;
    if (color == Cell::BLACK) {
        blackCaptured += 2;
        return;
    }
    whiteCaptured += 2;
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
