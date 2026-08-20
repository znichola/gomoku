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
    playMove(forceMoveId, true, false);
}

bool Board::playMove(unsigned id, bool forceMove, bool verifyComplete) {
    if (!forceMove && !isValidMove(id))
        return false;

    grid.set(id, isBlackToPlay ? Cell::BLACK : Cell::WHITE);

    std::vector<unsigned> removedCells = doCaptures(id);

    Cell victory = isVictoryNear(id, removedCells);
    winByCaptures = false;
    winByAlignment = false;
    winningCells.clear();
    if (verifyComplete) {
        VictoryInfo info = checkVictory();
        if (info.winner != victory) {
            COUT << "[GridTraversal] DIVERGENCE on move " << id
                 << ": isVictoryNear()=" << static_cast<int>(victory)
                 << " but full isVictory()=" << static_cast<int>(info.winner) << std::endl;
            MQ << "[GridTraversal] DIVERGENCE on move " << id
               << ": near=" << static_cast<int>(victory)
               << " full=" << static_cast<int>(info.winner) << "\n";
        }
        victory = info.winner;
        winByCaptures = info.byCaptures;
        winByAlignment = info.byAlignment;
        winningCells = std::move(info.winningCells);
    }
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

bool Board::playMove(unsigned id, bool verifyComplete) {
    return playMove(id, false, verifyComplete);
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
    return checkVictory().winner;
}

VictoryInfo Board::checkVictory() const {
    VictoryInfo info;
    if (isBlackToPlay && blackCaptured >= 10) {
        info.winner = Cell::BLACK;
        info.byCaptures = true;
    } else if (!isBlackToPlay && whiteCaptured >= 10) {
        info.winner = Cell::WHITE;
        info.byCaptures = true;
    }

    std::vector<unsigned> winningCells = grid.getWinningLineCells();
    if (!winningCells.empty()) {
        info.byAlignment = true;
        if (info.winner == Cell::EMPTY) info.winner = grid[winningCells.front()];
        info.winningCells = std::move(winningCells);
    }
    return info;
}

/**
 * Same result as isVictory(), but uses getWinningLineColorNearAround(id) instead of a
 * full-board scan - only valid when `id` is known to be the stone whose placement might have
 * just created the win (i.e. called from playMove with the move just played, or with
 * board.lastMove on a board that only ever went through playMove - never on a board loaded
 * from an arbitrary raw grid, where there's no reliable "last move" to check around).
 *
 * `removedCells` should be whatever doCaptures(id) returned for that same move: a capture
 * can revive a five-in-a-row elsewhere on the board just like the placement itself can (see
 * getWinningLineColorNearAround) - without checking around every removed cell too, that
 * revival goes undetected.
 */
Cell Board::isVictoryNear(unsigned id, const std::vector<unsigned> &removedCells) const {
    if ((isBlackToPlay && blackCaptured >= 10)) return Cell::BLACK;
    else if ((!isBlackToPlay && whiteCaptured >= 10)) return Cell::WHITE;
    if (id == FIRSTMOVE || id >= grid.size) return Cell::EMPTY;
    Cell winner = grid.getWinningLineColorNearAround(id);
    if (winner != Cell::EMPTY) return winner;
    for (unsigned removed : removedCells) {
        winner = grid.getWinningLineColorNearAround(removed);
        if (winner != Cell::EMPTY) return winner;
    }
    return Cell::EMPTY;
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
    out << "\"winByCaptures\": " << (winByCaptures ? "true" : "false") << ",\n";
    out << "\"winByAlignment\": " << (winByAlignment ? "true" : "false") << ",\n";
    out << "\"winningCells\": [";
    for (size_t i = 0; i < winningCells.size(); ++i) {
        out << winningCells[i];
        if (i + 1 < winningCells.size()) out << ",";
    }
    out << "],\n";
    out << "\"grid\": " << grid.serialize() << "\n";
    out << "}";

    return out.str();
}
