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

    long c = grid.handleCaptures(id, true);
    for (long i = 0; i < c; ++i)
        addCapture(myColor);
}

void Board::addCapture(Cell color) {
    if (color == Cell::BLACK) {
        blackCaptured += 2;
    } else if (color == Cell::WHITE) {
        whiteCaptured += 2;
    }
}

Cell Board::isVictory() {
    if ((isBlackToPlay && blackCaptured >= 10)) return Cell::BLACK;
    else if ((!isBlackToPlay && whiteCaptured >= 10)) return Cell::WHITE;

    return grid.getWinningLineColor();
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
