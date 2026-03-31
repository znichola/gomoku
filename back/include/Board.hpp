#pragma once

#include <vector>
#include <string>
#include <cmath>

#include "Cell.hpp"
#include "Grid.hpp"

struct Board {
    Grid grid;
    unsigned blackCaptured = 0;
    unsigned whiteCaptured = 0;
    bool isBlackToPlay = false;


    Board(unsigned board_size=19) : grid(Grid{board_size}) {}
    Board(const std::vector<Cell> &grid) : grid(std::move(grid)) {}
    Board(const Grid &grid) : grid(grid) {}

    bool playMove(unsigned id);
    void doCaptures(unsigned id);
    bool isValidMove(unsigned id);
    bool isVictory(unsigned id);
    void addCapture(Cell color);

    std::string serialize() const;
};
