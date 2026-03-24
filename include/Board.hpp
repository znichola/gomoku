#pragma once

#include <vector>
#include <string>

struct Board {
    enum class Cell: uint8_t {EMPTY=0, BLACK=1, WHITE=2};

    std::vector<Cell> grid;
    unsigned blackCaptured;
    unsigned whiteCaptured;
    bool isBlackToPlay = false;


    Board(unsigned board_size=19) : grid(board_size * board_size, Cell::EMPTY) {}

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

    inline Cell getXY(int x, int y);
    inline void setXY(int x, int y);

    std::string serialize() const; // someng like this or << overload
};
