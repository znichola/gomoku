#pragma once

#include <vector>
#include <string>

#include "Cell.hpp"

struct Grid {
    std::vector<Cell> grid;
    unsigned boardDimentions;

    Grid(unsigned board_size=19) : grid(board_size * board_size, Cell::EMPTY), boardDimentions(board_size) {}
    Grid(const std::vector<Cell> &grid) : grid(std::move(grid)), boardDimentions(static_cast<unsigned>(std::sqrt(grid.size()))) {}

    Cell operator[](unsigned id) const { return grid[id]; }
    Cell& operator[](unsigned id) { return grid[id]; }
    size_t size() const { return grid.size(); }

    std::string serialize() const;

    Grid &setBlack(unsigned id);
    Grid &setBlack(std::initializer_list<unsigned> ids);
    Grid &setWhite(unsigned id);
    Grid &setWhite(std::initializer_list<unsigned> ids);
    Grid &setEmpty(unsigned id);

    unsigned threeFreesPlayedPieceIsPartOf(unsigned id) const;
};