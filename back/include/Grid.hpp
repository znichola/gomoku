#pragma once

#include <vector>
#include <string>
#include <set>

#include "Cell.hpp"
#include "GridTraversal.hpp"
#include "Vector2D.hpp"

class GridTraversal;
class Grid {
private:
    std::vector<Cell> grid;
    GridTraversal *gridTraversal = nullptr;

public:
    unsigned width;
    unsigned size;

    Grid(unsigned width=19);
    Grid(const Grid &grid);

    ~Grid();

    void cleanCache();

	const std::vector<Cell>& getGrid() const;
    void setGrid(const std::vector<Cell>& newGrid);

    Cell operator[](unsigned id) const;
    Cell& operator[](unsigned id);

    GridTraversal const& nodes();

    Vector2D idToVec(unsigned id) const;
    unsigned vecToId(const Vector2D &vec) const;
    bool isInside(const Vector2D& vec) const;

    std::string serialize() const;

    Grid &setBlack(unsigned id);
    Grid &setBlack(std::initializer_list<unsigned> ids);
    Grid &setWhite(unsigned id);
    Grid &setWhite(std::initializer_list<unsigned> ids);
    Grid &setEmpty(unsigned id);

    Cell getWinningLineColor() const;
    long handleCaptures(unsigned const id, bool const apply = false);
    long calcAlignedCells(unsigned const id, long const ext, Cell &bc,
                    std::set<long> *alignedCells, long const offset = 0, long count = 0) const;
    bool isDoubleThree(unsigned const id) const;
};