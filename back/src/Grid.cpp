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

bool Grid::isDoubleThree(unsigned id) const {
    const Cell myColor = grid[id];
    if (myColor == Cell::EMPTY) return false;
    const Cell enemyColor = (myColor == Cell::BLACK ? Cell::WHITE : Cell::BLACK);

    const unsigned d = boardDimension;
    const std::initializer_list<Coord> extremities = {
        { 1,  0}, // right
        { 1,  1}, // bottom-right
        { 0,  1}, // bottom
        {-1,  1}, // bottom-left
        {-1,  0}, // left
        {-1, -1}, // top-left
        { 0, -1}, // top
        { 1, -1}  // top-right
    };

    const long cx = id % d;
    const long cy = id / d;
    for (long i = 0, nid = 0; i < 4; i++) {
        const char* spinner[] = {"—", "\\", "|", "/"};
        long l = 0;
        Cell lc = Cell::OUTSIDE;
        do {
            const long ox = (extremities.begin() + i)->first;
            const long oy = (extremities.begin() + i)->second;
            const long nx = cx + ox * (l + 1);
            const long ny = cy + oy * (l + 1);
            if (!(0 <= nx && nx < d && 0 <= ny && ny < d)) break;
            nid = ny * d + nx;
            lc = grid[nid];
            if (lc != myColor) break;
        } while (++l <= d);
        long r = 0;
        Cell rc = Cell::OUTSIDE;
        do { // TODO: CAN BE REFACTORED
            const long ox = (extremities.begin() + i + 4)->first;
            const long oy = (extremities.begin() + i + 4)->second;
            const long nx = cx + ox * (r + 1);
            const long ny = cy + oy * (r + 1);
            if (!(0 <= nx && nx < d && 0 <= ny && ny < d)) break;
            nid = ny * d + nx;
            rc = grid[nid];
            if (rc != myColor) break;
        } while (++r <= d);
        if (l + r >= 2) {
            std::cout << "[" << (myColor == Cell::BLACK ? "BLACK" : "WHITE") << "] ";
            std::cout << "[DIR: " << spinner[i % 4] << "] ";
        }
        if (l + r >= 4) {
            std::cout << l << " + " << r << " + 1 = " << (l+r+1) << " it is a win ! (98%, 2\% for capture)" << std::endl;
            // TODO: CHECK FOR CAPTURE
        } else if (l + r == 3 && lc == Cell::EMPTY && rc == Cell::EMPTY) {
            std::cout << l << " + " << r << " + 1 = " << (l+r+1) << " you will win ! Cannot be blocked !" << std::endl;
        } else if (l + r == 3) {
            std::cout << l << " + " << r << " + 1 = " << (l+r+1) << " you will win !? Not sure because can be blocked :(" << std::endl;
        } else if (l + r == 2 && lc == Cell::EMPTY && rc == Cell::EMPTY) {
            std::cout << l << " + " << r << " + 1 = " << (l+r+1) << " it is a free-three !?" << std::endl;
            // CHECK DOUBLE THREE HERE
        }
        // std::cout << "LR " << l << " " << r << std::endl;
    }

    (void) d;
    (void) extremities;
    (void) enemyColor;

    return false;
}