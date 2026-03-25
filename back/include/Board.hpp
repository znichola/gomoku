#pragma once

#include <vector>
#include <string>

struct Board {
    enum class Cell: uint8_t {EMPTY=0, BLACK=1, WHITE=2};

    std::vector<Cell> grid;
    unsigned boardSize;
    unsigned blackCaptured = 0;
    unsigned whiteCaptured = 0;
    bool isBlackToPlay = false;


    Board(unsigned board_size=19) :  grid(board_size * board_size, Cell::EMPTY), boardSize(board_size) {}
    Board(std::vector<Cell> grid, unsigned board_size) : grid(std::move(grid)), boardSize(board_size) {}

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
    bool isValidMove(unsigned id) const;

    std::string serialize() const;
};
