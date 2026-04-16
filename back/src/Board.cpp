#include <sstream>
#include <iostream>

#include "Board.hpp"
#include "Cell.hpp"
#include "MessageQueue.hpp"

bool Board::playMove(unsigned id) {
    if (!isValidMove(id))
        return false;

    grid[id] = isBlackToPlay ? Cell::BLACK : Cell::WHITE;

    doCaptures(id);

    Cell victory = isVictory();
    if (victory == Cell::OUTSIDE) {
        std::cout << "It's a draw" << std::endl;
        MQ << "It's a draw" << "\n";
    } else if (victory != Cell::EMPTY) {
        std::cout << "It's a win for " << (victory == Cell::BLACK ? "BLACK" : "WHITE") << std::endl;
        MQ << "It's a win for " << (victory == Cell::BLACK ? "BLACK" : "WHITE") << "\n";
    }

    isBlackToPlay = !isBlackToPlay;
    return true;
}

bool Board::isValidMove(unsigned id) {
    if (id >= grid.size())
        return false;

    if (grid[id] != Cell::EMPTY)
        return false;

    if (isBlackToPlay) grid.setBlack(id); else grid.setWhite(id);

    if (grid.isDoubleThree(id)) {
        grid.setEmpty(id);
        return false;
    }

    return true;
}

void Board::doCaptures(unsigned id) {
    const Cell myColor = grid[id];
    if (myColor == Cell::EMPTY) return;
    const Cell enemyColor = (myColor == Cell::BLACK ? Cell::WHITE : Cell::BLACK);
    const unsigned d = grid.boardDimension;
    const std::initializer_list<Coord> extremities = {
        { 1,  0}, // right
        { 1,  1}, // bottom-right
        { 0,  1}, // bottom
        {-1,  1}, // bottom-left
        {-1,  0}, // left
        {-1, -1}, // top-left
        { 0, -1}, // top
        { 1, -1}  // top-right
    };

    const long cx = id % d;
    const long cy = id / d;
    for (auto [ox, oy] : extremities) {
        const long nx = cx + ox * 3;
        const long ny = cy + oy * 3;
        const long nid = ny * d + nx;
        if (!(0 <= nx && nx < d && 0 <= ny && ny < d)) continue;
        if (grid[nid] != myColor) continue;
        const long nid1 = (cy + oy * 1) * d + (cx + ox * 1);
        if (grid[nid1] != enemyColor) continue;
        const long nid2 = (cy + oy * 2) * d + (cx + ox * 2);
        if (grid[nid2] != enemyColor) continue;
        grid.setEmpty(static_cast<unsigned>(nid1));
        grid.setEmpty(static_cast<unsigned>(nid2));
        addCapture(myColor);
    }
}

void Board::addCapture(Cell color) {
    if (color == Cell::EMPTY) return;
    if (color == Cell::BLACK) {
        blackCaptured += 2;
        return;
    }
    whiteCaptured += 2;
}

Cell Board::isVictory() {
    if ((isBlackToPlay && blackCaptured >= 10)) return Cell::BLACK;
    else if ((!isBlackToPlay && whiteCaptured >= 10)) return Cell::WHITE;

    // check 5 in a row
    /// check no captures possible on 5 in a row

    return Cell::EMPTY;
}

std::string Board::serialize() const {
    std::ostringstream out;

    out << "{\n";
    out << "\"boardDimension\": " << grid.boardDimension << ",\n";
    out << "\"blackCaptured\": " << blackCaptured << ",\n";
    out << "\"whiteCaptured\": " << whiteCaptured << ",\n";
    out << "\"isBlackToPlay\": " << (isBlackToPlay ? "true" : "false") << ",\n";
    out << "\"grid\": " << grid.serialize() << "\n";
    out << "}";

    return out.str();
}
