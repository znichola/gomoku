#include <sstream>
#include <cmath>
#include <iostream>

#include "Grid.hpp"

std::string Grid::serialize() const {
    std::ostringstream out;
    out << "[";
    for (size_t i = 0; i < grid.size(); ++i) {
        out << static_cast<int>(grid[i]);
        if (i + 1 < grid.size())
            out << ",";
    }
    out << "]\n";
    return out.str();
}

Grid &Grid::setBlack(unsigned id) {
    grid[id] = Cell::BLACK;
    return *this;
}

Grid &Grid::setBlack(std::initializer_list<unsigned> ids) {
    for (auto id : ids) {
        grid[id] = Cell::BLACK;
    }
    return *this;
}


Grid &Grid::setWhite(unsigned id) {
    grid[id] = Cell::WHITE;
    return *this;
}

Grid &Grid::setWhite(std::initializer_list<unsigned> ids) {
    for (auto id : ids) {
        grid[id] = Cell::WHITE;
    }
    return *this;
}

Grid &Grid::setEmpty(unsigned id) {
    grid[id] = Cell::EMPTY;
    return *this;
}

unsigned Grid::threeFreesPlayedPieceIsPartOf(unsigned id) const {
    if (grid[id] == Cell::EMPTY)
        return 0;

    Cell myColor = grid[id];
    Cell opponentColor = (myColor == Cell::BLACK) ? Cell::WHITE : Cell::BLACK;

    auto getHorizontalTiles = [this, opponentColor](unsigned id) -> std::vector<Cell> {
        long idl = static_cast<long>(id);
        auto ids = {idl-5, idl-4, idl-3, idl-2, idl-1, idl, idl+1, idl+2, idl+3, idl+4, idl+5};
        std::vector<Cell> tiles;
        for (long i : ids) {
            if (i < 0 || i >= static_cast<long>(grid.size()) || true /*some check so ids stay in the same line*/)
                tiles.push_back(opponentColor);
            else
                tiles.push_back(grid[i]);
        }
        return tiles;
    };

    auto horizontalTiles = getHorizontalTiles(id);
    // std::cout << "Horizontal tiles for id " << id << ": ";
    // for (const auto& tile : horizontalTiles) {
    //     std::cout << static_cast<int>(tile) << " ";
    // }
    // std::cout << "\n";

    if (horizontalTiles[4] == opponentColor || horizontalTiles[6] == opponentColor)
        return 0;

    /*
        Horizontal checks

            |        
    0 0 0 0 x 0 0 0 0 res NO
    0 0 0 0 x x 0 0 0 res NO
    0 0 0 x x 0 0 0 0 res NO
    0 0 0 x x x 0 0 0 res YES
    0 0 x 0 x 0 x 0 0 res YES
    0 0 0 0 x 0 x x 0 res YES
    0 x x 0 x 0 0 0 0 res YES
    0 0 x x x 0 0 0 0 res YES


if any of the two flanking pieces are opposit color, it's NO
    0 0 0 0 0 x w 0 0 0 0 res NO
    0 0 0 0 w x 0 0 0 0 0 res NO
    0 0 0 0 w x x x 0 0 0 res NO

    0 0 0 W 0 x 0 x x 0 0 res NO

    1 2 3 4 5 6 7 8 9 10 11
              |          
    0 0 0 w 0 x x x 0 0 0 res YES
    0 0 0 w 0 x x x 0 W 0 res NO
    0 0 0 w 0 x x x 0 W 0 res YES
    0 0 0 w 0 x 0 x x W 0 res NO

    */


    /*

011100
010110
010101
001110
001101
000111
100111

[1, 1, 1, 0, 0]
[1, 0, 1, 1, 0]
[1, 0, 1, 0, 1]
[0, 1, 1, 1, 0]
[0, 1, 1, 0, 1]
[0, 0, 1, 1, 1]
    
    */

    return 0;
}