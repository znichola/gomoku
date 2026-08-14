#include <sstream>
#include <iostream>

#include "Board.hpp"
#include "Cell.hpp"
#include "MessageQueue.hpp"

Board::Board(unsigned width) {
    grid = Grid{width};
}

Board::Board(const Grid &grid) : grid(grid) {

}

Board::Board(const Board &board, unsigned forceMoveId) : Board(board) {
    playMove(forceMoveId, true);
}

bool Board::playMove(unsigned id, bool forceMove) {
    if (!forceMove && !isValidMove(id))
        return false;

    grid.set(id, isBlackToPlay ? Cell::BLACK : Cell::WHITE);

    std::vector<unsigned> removedCells = doCaptures(id);

    Cell victory = isVictoryNear(id);
    if (victory == Cell::OUTSIDE) {
        COUT << "It's a draw" << std::endl;
        MQ << "It's a draw" << "\n";
    } else if (victory != Cell::EMPTY) {
        COUT << "It's a win for " << (victory == Cell::BLACK ? "BLACK" : "WHITE") << std::endl;
        MQ << "It's a win for " << (victory == Cell::BLACK ? "BLACK" : "WHITE") << "\n";
    }
    winner = victory;

    lastMove = id;
    isBlackToPlay = !isBlackToPlay;
    return true;
}

bool Board::playMove(unsigned id) {
    return playMove(id, false);
}

bool Board::isValidMove(unsigned id) const {
    if (id >= grid.size)
        return false;

    if (grid[id] != Cell::EMPTY)
        return false;

    if (winner != Cell::EMPTY)
        return false;

    if (grid.isDoubleThree(id, isBlackToPlay ? Cell::BLACK : Cell::WHITE))
        return false;

    return true;
}

std::vector<unsigned> Board::doCaptures(unsigned id) {
    const Cell myColor = grid[id];
    if (myColor == Cell::EMPTY) return {};

    std::vector<unsigned> removedCells;
    long c = grid.handleCaptures(id, true, &removedCells);
    for (long i = 0; i < c; ++i)
        addCapture(myColor);
    return removedCells;
}

void Board::addCapture(Cell color) {
    if (color == Cell::BLACK) {
        blackCaptured += 2;
    } else if (color == Cell::WHITE) {
        whiteCaptured += 2;
    }
}

Cell Board::isVictory() const {
    if ((isBlackToPlay && blackCaptured >= 10)) return Cell::BLACK;
    else if ((!isBlackToPlay && whiteCaptured >= 10)) return Cell::WHITE;
    return grid.getWinningLineColor();
}

/**
 * Same result as isVictory(), but uses getWinningLineColorNear(id) instead of a full-board
 * scan - only valid when `id` is known to be the stone whose placement might have just
 * created the win (i.e. called from playMove with the move just played, or with
 * board.lastMove on a board that only ever went through playMove - never on a board loaded
 * from an arbitrary raw grid, where there's no reliable "last move" to check around).
 */
Cell Board::isVictoryNear(unsigned id) const {
    if ((isBlackToPlay && blackCaptured >= 10)) return Cell::BLACK;
    else if ((!isBlackToPlay && whiteCaptured >= 10)) return Cell::WHITE;
    if (id == FIRSTMOVE || id >= grid.size) return Cell::EMPTY;
    return grid.getWinningLineColorNear(id);
}

bool Board::isGameOver() const {
    return Cell::EMPTY != isVictory();
}

std::string Board::serialize() const {
    std::ostringstream out;

    out << "{\n";
    out << "\"width\": " << grid.width << ",\n";
    out << "\"blackCaptured\": " << blackCaptured << ",\n";
    out << "\"whiteCaptured\": " << whiteCaptured << ",\n";
    out << "\"isBlackToPlay\": " << (isBlackToPlay ? "true" : "false") << ",\n";
    out << "\"winner\": " << static_cast<int>(winner) << ",\n";
    out << "\"grid\": " << grid.serialize() << "\n";
    out << "}";

    return out.str();
}
