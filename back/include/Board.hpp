#pragma once

#include <vector>
#include <string>
#include <cmath>

#include "Cell.hpp"
#include "Grid.hpp"

struct Board {
    static constexpr unsigned FIRSTMOVE = 0xFFFF;

    Grid grid;
    unsigned blackCaptured = 0;
    unsigned whiteCaptured = 0;
    bool isBlackToPlay = true;
    Cell winner = Cell::EMPTY;
    unsigned lastMove = FIRSTMOVE; // TODO only half hartidly implemented, not sure about state reset etc

    Board(unsigned width=19);
    Board(const Grid &grid);

    bool playMove(unsigned id);
    void doCaptures(unsigned id);
    bool isValidMove(unsigned id);
    Cell isVictory() const;
    bool isGameOver() const;
    void addCapture(Cell color);

    std::string serialize() const;
};
