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

    // maybe later
    // std::set<size_t> _whitePieces;
    // std::set<size_t> _blackPieces;


    /*
    y   x y
    0   0 1 2 3 4
    1   5 6 7 8 9

    y = id / WIDTH
    x = (id ) % WIDTH
    id = y * WIDTH + x
    */

    bool playMove(unsigned id);
    void doCaptures(unsigned id);
    bool isValidMove(unsigned id);
    bool isVictory(unsigned id);
    void addCapture(Cell color);

    std::string serialize() const;
};
