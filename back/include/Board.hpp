#pragma once

#include <vector>
#include <string>
#include <cmath>

#include "Cell.hpp"
#include "Grid.hpp"

struct VictoryInfo {
    Cell winner = Cell::EMPTY;
    bool byCaptures = false;
    bool byAlignment = false;
    std::vector<unsigned> winningCells;
};

class Board {
private:
    bool playMove(unsigned id, bool forceMove, bool verifyComplete);

public:
    static constexpr unsigned FIRSTMOVE = 0xFFFF;

    Grid grid;
    unsigned blackCaptured = 0;
    unsigned whiteCaptured = 0;
    bool isBlackToPlay = true;
    Cell winner = Cell::EMPTY;
    bool winByCaptures = false;
    bool winByAlignment = false;
    std::vector<unsigned> winningCells;
    unsigned lastMove = FIRSTMOVE; // TODO only half hartidly implemented, not sure about state reset etc

    Board(unsigned width=19);
    Board(const Grid &grid);
    /**
     * @param forceMoveId is NOT verified with isValidMove
     */
    Board(const Board &board, unsigned forceMoveId);

    bool playMove(unsigned id, bool verifyComplete = false);
    std::vector<unsigned> doCaptures(unsigned id);
    bool isValidMove(unsigned id) const;
    Cell isVictory() const;
    VictoryInfo checkVictory() const;
    Cell isVictoryNear(unsigned id, const std::vector<unsigned> &removedCells = {}) const;
    bool isGameOver() const;
    void addCapture(Cell color);

    std::string serialize() const;
};
