#pragma once

#include <vector>
#include <string>
#include <set>

#include "Cell.hpp"
#include "Vector2D.hpp"

extern const std::initializer_list<Vector2D> extremities;

struct Grid {
    std::vector<Cell> grid;
    unsigned boardDimension;

    Grid(unsigned board_size=19) : grid(board_size * board_size, Cell::EMPTY), boardDimension(board_size) {}
    Grid(const std::vector<Cell> &grid) : grid(std::move(grid)), boardDimension(static_cast<unsigned>(std::sqrt(grid.size()))) {}

    Cell operator[](unsigned id) const { return grid[id]; }
    Cell& operator[](unsigned id) { return grid[id]; }
    size_t size() const { return grid.size(); }

    std::string serialize() const;

    Grid &setBlack(unsigned id);
    Grid &setBlack(std::initializer_list<unsigned> ids);
    Grid &setWhite(unsigned id);
    Grid &setWhite(std::initializer_list<unsigned> ids);
    Grid &setEmpty(unsigned id);

    Cell getWinningLineColor() const;
    long handleCaptures(unsigned const id, bool const apply = false);
    long calcAlignedCells(unsigned const id, long const i, Cell &bc,
                    std::set<long> *alignedCells, long const offset = 0, long c = 0) const;
    bool isDoubleThree(unsigned const id) const;
};