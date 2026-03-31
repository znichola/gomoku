#include <sstream>
#include <cmath>
#include <iostream>
#include <set>

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

struct Vector2D {
    long x, y;

    // Constructeur
    Vector2D(long x = 0, long y = 0) : x(x), y(y) {}

    // Addition
    Vector2D operator+(const Vector2D& other) const {
        return {x + other.x, y + other.y};
    }

    // Soustraction
    Vector2D operator-(const Vector2D& other) const {
        return {x - other.x, y - other.y};
    }

    // Multiplication par un scalaire
    Vector2D operator*(long k) const {
        return {x * k, y * k};
    }

    // Produit scalaire
    long dot(const Vector2D& other) const {
        return x * other.x + y * other.y;
    }

    // Déterminant (utile pour colinéarité)
    long cross(const Vector2D& other) const {
        return x * other.y - y * other.x;
    }

    // Affichage
    friend std::ostream& operator<<(std::ostream& os, const Vector2D& v) {
        return os << "(" << v.x << ", " << v.y << ")";
    }
};

bool Grid::isDoubleThree(unsigned id) const {
    const Cell myColor = grid[id];
    if (myColor == Cell::EMPTY) return false;
    const Cell enemyColor = (myColor == Cell::BLACK ? Cell::WHITE : Cell::BLACK);

    const unsigned d = boardDimension;
    const std::initializer_list<Coord> extremities = { // TODO: CAN BE A GLOBAL
        { 1,  0}, // right
        { 1,  1}, // bottom-right
        { 0,  1}, // bottom
        {-1,  1}, // bottom-left
        {-1,  0}, // left
        {-1, -1}, // top-left
        { 0, -1}, // top
        { 1, -1}  // top-right
    };

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
        c++;
    }
    if (c > 0) {
        std::cout << "It is a capture ! (" << c << ")" << std::endl;
    }

    for (long i = 0, nid = 0; i < 4; i++) {
        std::set<long> alignedCells = { id };
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
            alignedCells.insert(nid);
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
            alignedCells.insert(nid);
        } while (++r <= d);
        if (l + r >= 2) {
            std::cout << "[" << (myColor == Cell::BLACK ? "BLACK" : "WHITE") << "] ";
            std::cout << "[DIR: " << spinner[i % 4] << "] ";
        }
        if (l + r >= 4) {
            std::cout << l << " + " << r << " + 1 = " << (l+r+1) << " it is a win ! (98%, 2\% for capture)" << std::endl;
            // TODO: ~~CHECK FOR CAPTURE~~ DO WE REALLY NEED IT ?
        } else if (l + r == 3 && lc == Cell::EMPTY && rc == Cell::EMPTY) {
            std::cout << l << " + " << r << " + 1 = " << (l+r+1) << " you will win ! Cannot be blocked !" << std::endl;
        } else if (l + r == 3) {
            std::cout << l << " + " << r << " + 1 = " << (l+r+1) << " you will win !? Not sure because can be blocked :(" << std::endl;
        } else if (l + r == 2 && lc == Cell::EMPTY && rc == Cell::EMPTY) {
            // std::cout << l << " + " << r << " + 1 = " << (l+r+1) << " it is a free-three." << std::endl;
            MQ << "IT's a single free three";
            return 1;

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
                const long acx = acid % d;
                const long acy = acid / d;
                // const Vector2D p3 = { acid % d, acid / d };
                // const Vector2D v2 = p3 - p1;
                if (alignedCells.find(acid) != alignedCells.end())
                    continue; // WE DON'T WANT ALIGNED CELLS ONLY ADJACENT FROM THE LINE OF ALIGNED CELLS

                // const long cross = v1.cross(v2); // DETERMINANT = COLINEARITE = SUR LA MEME DROITE
                // if (cross == 0) {
                //     continue;
                // }

                for (long j = 0, nid2 = 0; j < 4; j++) {
                    if (i == j) continue;
                    long l2 = 0;
                    Cell lc2 = Cell::OUTSIDE;
                    do {
                        const long ox = (extremities.begin() + j)->first;
                        const long oy = (extremities.begin() + j)->second;
                        const long nx = acx + ox * (l2 + 1);
                        const long ny = acy + oy * (l2 + 1);
                        if (!(0 <= nx && nx < d && 0 <= ny && ny < d)) break;
                        nid2 = ny * d + nx;
                        lc2 = grid[nid2];
                        if (lc2 != myColor) break;
                    } while (++l2 <= d);
                    long r2 = 0;
                    Cell rc2 = Cell::OUTSIDE;
                    do { // TODO: CAN BE REFACTORED
                        const long ox = (extremities.begin() + j + 4)->first;
                        const long oy = (extremities.begin() + j + 4)->second;
                        const long nx = acx + ox * (r2 + 1);
                        const long ny = acy + oy * (r2 + 1);
                        if (!(0 <= nx && nx < d && 0 <= ny && ny < d)) break;
                        nid2 = ny * d + nx;
                        rc2 = grid[nid2];
                        if (rc2 != myColor) break;
                    } while (++r2 <= d);
                    if (l2 + r2 > 2) {
                        std::cout << "[" << acid << "] ";
                        std::cout << "[DIR: " << spinner[j % 4] << "] [SCORE: " << (l2+r2+1) << "] ";
                        if (lc2 == Cell::EMPTY && rc2 == Cell::EMPTY) {
                            std::cout << l2 << " + " << r2 << " + 1 = " << (l2+r2+1) << " it is a DOUBLE free-three." << std::endl;
                            MQ << "IT's a dobble free three";
                            return true; // WE END AT FIRST DFT FOUND
                        } else {
                            std::cout << std::endl;
                        }
                    }
                }
            }
        }
    }

    return false;
}