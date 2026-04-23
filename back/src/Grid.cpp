#include <sstream>
#include <cmath>
#include <iostream>

#include "Grid.hpp"
#include "MessageQueue.hpp"
#include "Utils.hpp"
#include <assert.h>

Grid::Grid(unsigned width) : grid(width * width, Cell::EMPTY), width(width), size(width * width) {

}

Grid::Grid(const Grid &grid) : grid(std::move(grid.grid)), width(grid.width), size(width * width) {

}

Grid::~Grid() {
    if (gridTraversal != nullptr)
        delete gridTraversal;
}

void Grid::cleanCache() {
    if (gridTraversal != nullptr) {
        gridTraversal = nullptr;
    }
}

const std::vector<Cell>& Grid::getGrid() const {
    return grid;
}

void Grid::setGrid(const std::vector<Cell>& newGrid) {
    grid = newGrid;
    cleanCache();
    generateHash();
}

Cell Grid::operator[](unsigned id) const {
    return grid[id];
}

Grid &Grid::set(unsigned id, Cell value) {
    cleanCache();
    updateHash(id, value);
    grid[id] = value;
    return *this;
}

/* Cell& Grid::operator[](unsigned id) {
    cleanCache();
    generateHash();
    return grid[id];
} */

Grid &Grid::setBlack(unsigned id) {
    return set(id, Cell::BLACK);
}

Grid &Grid::setBlack(std::initializer_list<unsigned> ids) {
    for (auto id : ids) {
        set(id, Cell::BLACK);
    }
    return *this;
}

Grid &Grid::setWhite(unsigned id) {
    return set(id, Cell::WHITE);
}

Grid &Grid::setWhite(std::initializer_list<unsigned> ids) {
    for (auto id : ids) {
        set(id, Cell::WHITE);
    }
    return *this;
}

Grid &Grid::setEmpty(unsigned id) {
    return set(id, Cell::EMPTY);
    return *this;
}

void Grid::generateHash() {
    uint64_t h = 0;
    for (unsigned id = 0; id < 361; ++id) {
        h ^= zob[id][static_cast<int>(grid[id])];
    }
    hash = h;
}

uint64_t Grid::getHash() const {
    return hash;
}

void Grid::updateHash(unsigned id, Cell newValue) {
    hash ^= zob[id][static_cast<int>(grid[id])];
    hash ^= zob[id][static_cast<int>(newValue)];
}

const GridTraversal& Grid::nodes() const {
    if (gridTraversal != nullptr) 
        return *gridTraversal;

    auto it = tableGridTraversal.find(hash);
    if (it != tableGridTraversal.end()
            // && memcmp(it->second.grid.getGrid().data(), grid.getGrid().data(), 361) == 0
            ) {
        return it->second;
    } else {
        auto [it, inserted] = tableGridTraversal.try_emplace(hash, *this);
        return it->second;
    }
}

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

Vector2D Grid::idToVec(unsigned id) const {
    assert(id < grid.size());
    return Vector2D::createFromIndex(id, width);
}

unsigned Grid::vecToId(const Vector2D& vec) const {
    assert(isInside(vec));
    return vec.toIndex(width);
}

bool Grid::isInside(const Vector2D& vec) const {
    return vec >= 0 && vec < width;
}

Cell Grid::getWinningLineColor() const {
    const GridTraversal& gridTraversal = this->nodes();
    for (const NodeCellRow& cellRow : gridTraversal.getCellRowsGarbage()) {
        if (cellRow.score >= 5 && cellRow.type != Cell::EMPTY) {
            return cellRow.type;
        }
    }
    return Cell::EMPTY;
}

long Grid::detectCaptures(unsigned const id, const Cell myColor) const {
    const Cell enemyColor = (myColor == Cell::BLACK ? Cell::WHITE : Cell::BLACK);

    long c = 0;
    const Vector2D cellPoint = Vector2D::createFromIndex(id, width);
    for (Vector2D offsetPoint : EXTREMITIES) {
        const Vector2D newPoint = cellPoint + offsetPoint * 3;
        if (!isInside(newPoint)) continue;
        const long nid = newPoint.toIndex(width);
        if (grid[nid] != myColor) continue;
        const long nid1 = (cellPoint + offsetPoint).toIndex(width);
        if (grid[nid1] != enemyColor) continue;
        const long nid2 = (cellPoint + offsetPoint * 2).toIndex(width);
        if (grid[nid2] != enemyColor) continue;
        ++c;
    }
    return c;
}

/**
 * @param apply If false just read, without modify member variable
 * @return number of pairs captured (1 for two stones captured)
 */
long Grid::handleCaptures(unsigned const id, bool const apply) {
    const Cell myColor = grid[id];
    if (myColor == Cell::EMPTY) return 0;
    const Cell enemyColor = (myColor == Cell::BLACK ? Cell::WHITE : Cell::BLACK);

    long c = 0;
    const Vector2D cellPoint = Vector2D::createFromIndex(id, width);
    for (Vector2D offsetPoint : EXTREMITIES) {
        const Vector2D newPoint = cellPoint + offsetPoint * 3;
        if (!isInside(newPoint)) continue;
        const long nid = newPoint.toIndex(width);
        if (grid[nid] != myColor) continue;
        const long nid1 = (cellPoint + offsetPoint).toIndex(width);
        if (grid[nid1] != enemyColor) continue;
        const long nid2 = (cellPoint + offsetPoint * 2).toIndex(width);
        if (grid[nid2] != enemyColor) continue;
        if (apply) {
            setEmpty(static_cast<unsigned>(nid1));
            setEmpty(static_cast<unsigned>(nid2));
        }
        ++c;
    }
    return c;
}

long Grid::calcAlignedCells(unsigned const id, long const ext, Cell &bc,
                        std::set<long> *alignedCells, long const offset, long count) const {
    const Cell myColor = grid[id];
    if (myColor == Cell::EMPTY) return false;

    const Vector2D cellPoint = Vector2D::createFromIndex(id, width);
    const Vector2D offsetPoint = *(extptr + ext + offset);
    do {
        const Vector2D newPoint = cellPoint + offsetPoint * (count + 1);
        if (!isInside(newPoint)) break;
        const long nid = newPoint.toIndex(width);
        bc = grid[nid];
        if (bc != myColor) break;
        if (alignedCells) alignedCells->insert(nid);
    } while (++count <= width);
    return count;
}

bool Grid::isDoubleThree(unsigned const id, const Cell myColor) const {
    if (myColor == Cell::EMPTY) return false;

    long c = detectCaptures(id, myColor);
    if (c > 0) return false; // It's a capture

    c = 0;
    for (long ext = 0; ext < 4; ++ext) {
        // std::set<long> alignedCells = { id };

        Cell lc = Cell::OUTSIDE;
        long l = calcAlignedCells(id, ext, lc, NULL); //&alignedCells);

        Cell rc = Cell::OUTSIDE;
        long r = calcAlignedCells(id, ext, rc, NULL, 4); //&alignedCells, 4);

        bool specialThree = false;
        if (l + r <= 1 && lc == Cell::EMPTY && rc == Cell::EMPTY) {
            long needed = (l + r == 0) ? 2 : 1;
            Cell lc2 = Cell::OUTSIDE;
            long l2 = calcAlignedCells(id, ext, lc2, NULL, 0, l + 1) - l - 1;
            Cell rc2 = Cell::OUTSIDE;
            long r2 = calcAlignedCells(id, ext, rc2, NULL, 4, r + 1) - r - 1;
            if ((needed == l2 && lc2 == Cell::EMPTY) || (needed == r2 && rc2 == Cell::EMPTY)) {
                specialThree = true;
            }
        }
        if (specialThree) {
            ++c;
        } else if (l + r >= 3) {
            return false;
        } else if (l + r == 2 && lc == Cell::EMPTY && rc == Cell::EMPTY) {
            ++c;
        }
    }

    if (c > 1)
        return true;
    return false;
}
            /* JE NE SAIS PAS SI CE CODE PEUT NOUS SERVIR POUR L'IA ???
            std::set<long> alignedCells = { id };

            Cell lc = Cell::OUTSIDE;
            long l = calcAlignedCells(id, ext, lc, NULL, &alignedCells);

            Cell rc = Cell::OUTSIDE;
            long r = calcAlignedCells(id, ext, rc, NULL, 4, &alignedCells, 4);

            std::set<long> adjacentCells;

            for (auto acid : alignedCells) {
                const long acx = acid % d;
                const long acy = acid / d;
                for (auto [ox, oy] : EXTREMITIES) {
                    const long nx = acx + ox;
                    const long ny = acy + oy;
                    const long nid = ny * d + nx;
                    if (!(0 <= nx && nx < d && 0 <= ny && ny < d)) continue;
                    if (grid[nid] == myColor || grid[nid] == Cell::EMPTY) {
                        adjacentCells.insert(nid);
                    }
                }
            }
            // const long nid1 = *alignedCells.begin();
            // const long nid2 = *std::prev(alignedCells.cend());
            // const Vector2D p1 = { nid1 % d, nid1 / d };
            // const Vector2D p2 = { nid2 % d, nid2 / d };
            // const Vector2D v1 = p2 - p1;
            for (auto acid : adjacentCells) { // TODO: CAN BE REFACTORED
                // const Vector2D p3 = { acid % d, acid / d };
                // const Vector2D v2 = p3 - p1;
                if (alignedCells.find(acid) != alignedCells.end())
                    continue; // WE DON'T WANT ALIGNED CELLS ONLY ADJACENT FROM THE LINE OF ALIGNED CELLS

                // const long cross = v1.cross(v2); // DETERMINANT = COLINEARITE = SUR LA MEME DROITE
                // if (cross == 0) {
                //     continue;
                // }

                for (long ext2 = 0; ext2 < 4; ++ext2) {
                    if (ext == ext2) continue;

                    Cell lc2 = Cell::OUTSIDE;
                    long l2 = calcAlignedCells(acid, ext2, lc2, NULL);

                    Cell rc2 = Cell::OUTSIDE;
                    long r2 = calcAlignedCells(acid, ext2, rc2, NULL, 4);


                    if (l2 + r2 >= 2) {
                        std::cout << "[" << acid << "] ";
                        std::cout << "[" << (myColor == Cell::BLACK ? "B" : "W") << " " << SPINNER[ext2 % 4] << "] ";
                        std::cout << l2 << " + " << r2 << " + 1 = " << (l2+r2+1);
                        if (lc2 == Cell::EMPTY && rc2 == Cell::EMPTY && l2 + r2 == 2) {
                            std::cout << " it is a DOUBLE free-three." << std::endl;
                            return true; // WE END AT FIRST DFT FOUND
                        } else {
                            std::cout << std::endl;
                        }
                    }
                }
            }*/
