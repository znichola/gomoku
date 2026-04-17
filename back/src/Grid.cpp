#include <sstream>
#include <cmath>
#include <iostream>

#include "Grid.hpp"
#include "MessageQueue.hpp"

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

/**
 * @param apply If false just read, without modify member variable
 */
long Grid::handleCaptures(unsigned const id, bool const apply) {
    const Cell myColor = grid[id];
    if (myColor == Cell::EMPTY) return false;
    const Cell enemyColor = (myColor == Cell::BLACK ? Cell::WHITE : Cell::BLACK);
    const unsigned d = boardDimension;

    long c = 0;
    const long cx = id % d;
    const long cy = id / d;
    for (auto [ox, oy] : extremities) { // TODO: CAN BE REFACTORED with `void Board::doCaptures(unsigned id)`;
        const long nx = cx + ox * 3;
        const long ny = cy + oy * 3;
        const long nid = ny * d + nx;
        if (!(0 <= nx && nx < d && 0 <= ny && ny < d)) continue;
        if (grid[nid] != myColor) continue;
        const long nid1 = (cy + oy * 1) * d + (cx + ox * 1);
        if (grid[nid1] != enemyColor) continue;
        const long nid2 = (cy + oy * 2) * d + (cx + ox * 2);
        if (grid[nid2] != enemyColor) continue;
        if (apply) {
            setEmpty(static_cast<unsigned>(nid1));
            setEmpty(static_cast<unsigned>(nid2));
        }
        c++;
    }
    return c;
}

long Grid::calcAlignedCells(unsigned const id, long const i, Cell &bc,
                        std::set<long> *alignedCells, long const offset) const {
    const Cell myColor = grid[id];
    if (myColor == Cell::EMPTY) return false;
    const unsigned d = boardDimension;

    long c = 0;
    long nid = 0;
    const long cx = id % d;
    const long cy = id / d;
    do {
        const long ox = (extremities.begin() + i + offset)->first;
        const long oy = (extremities.begin() + i + offset)->second;
        const long nx = cx + ox * (c + 1);
        const long ny = cy + oy * (c + 1);
        if (!(0 <= nx && nx < d && 0 <= ny && ny < d)) break;
        nid = ny * d + nx;
        bc = grid[nid];
        if (bc != myColor) break;
        if (alignedCells) alignedCells->insert(nid);
    } while (++c <= d);
    return c;
}

bool Grid::isDoubleThree(unsigned const id) const {
    std::cout << "== isDoubleThree ==" << std::endl;
    const Cell myColor = grid[id];
    if (myColor == Cell::EMPTY) return false;

    long c = const_cast<Grid*>(this)->handleCaptures(id);
    if (c > 0) {
        std::cout << "It is a capture ! (" << c << ")" << std::endl;
    }

    const unsigned d = boardDimension;
    c = 0;
    for (long i = 0; i < 4; i++) {
        std::set<long> alignedCells = { id };

        Cell lc = Cell::OUTSIDE;
        long l = calcAlignedCells(id, i, lc, &alignedCells);

        Cell rc = Cell::OUTSIDE;
        long r = calcAlignedCells(id, i, rc, &alignedCells, 4);

        const char* spinner[] = {"—", "\\", "|", "/"};
        if (l + r >= 2) {
            std::cout << "[" << (myColor == Cell::BLACK ? "B" : "W") << " " << spinner[i % 4] << "] ";
            std::cout << l << " + " << r << " + 1 = " << (l+r+1);
        }
        if (l + r >= 4) {
            std::cout << " 5: Probably a win." << std::endl;
            return false;
        } else if (l + r == 3 && lc == Cell::EMPTY && rc == Cell::EMPTY) {
            std::cout << " 4: Cannot be blocked !" << std::endl;
        } else if (l + r == 3) {
            std::cout << " 4: Can be blocked :(" << std::endl;
        } else if (l + r == 2 && lc == Cell::EMPTY && rc == Cell::EMPTY) {
            std::cout << " 3: it is a free-three." << std::endl;
            c++;
            continue ;

            std::set<long> adjacentCells;

            for (auto acid : alignedCells) {
                const long acx = acid % d;
                const long acy = acid / d;
                for (auto [ox, oy] : extremities) {
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

                for (long j = 0; j < 4; j++) {
                    if (i == j) continue;

                    Cell lc2 = Cell::OUTSIDE;
                    long l2 = calcAlignedCells(acid, j, lc2, NULL);

                    Cell rc2 = Cell::OUTSIDE;
                    long r2 = calcAlignedCells(acid, j, rc2, NULL, 4);


                    if (l2 + r2 >= 2) {
                        std::cout << "[" << acid << "] ";
                        std::cout << "[" << (myColor == Cell::BLACK ? "B" : "W") << " " << spinner[j % 4] << "] ";
                        std::cout << l2 << " + " << r2 << " + 1 = " << (l2+r2+1);
                        if (lc2 == Cell::EMPTY && rc2 == Cell::EMPTY && l2 + r2 == 2) {
                            std::cout << " it is a DOUBLE free-three." << std::endl;
                            return true; // WE END AT FIRST DFT FOUND
                        } else {
                            std::cout << std::endl;
                        }
                    }
                }
            }
        } else if (l + r >= 2) {
            std::cout << std::endl;
        }
    }

    if (c > 1)
        return true;
    return false;
}
