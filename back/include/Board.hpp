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
    unsigned lastMove = FIRSTMOVE; // TODO only half hartidly implemented, not sure about state reset etc

    Board(unsigned board_size=19) : grid(Grid{board_size}) {}
    Board(const std::vector<Cell> &grid) : grid(std::move(grid)) {}
    Board(const Grid &grid) : grid(grid) {}

    bool playMove(unsigned id);
    void doCaptures(unsigned id);
    bool isValidMove(unsigned id);
    Cell isVictory() const;
    bool isGameOver() const;
    void addCapture(Cell color);

    std::string serialize() const;
};
