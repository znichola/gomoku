#include <sstream>
#include <cmath>
#include <iostream>
#include <deque>
#include <functional>

#include "Grid.hpp"
#include "MessageQueue.hpp"
#include <assert.h>

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

Vector2D Grid::idToVec(unsigned id) const {
    assert(id < grid.size());
    return Vector2D::createFromIndex(id, boardDimension);
}

unsigned Grid::vecToId(const Vector2D& vec) const {
    assert(isInside(vec));
    return vec.toIndex(boardDimension);
}

bool Grid::isInside(const Vector2D& vec) const {
    return vec >= 0 && vec < boardDimension;
}

// Level2Death : 1) LOOKING > 2) BOILING > 3) END
enum class NodeStep: char {
    NONE = 0U, // ALIVE
    LOOKING = 1U,
    BOILING = 2U,
    DEATH = 3U,
    END = 4U
};

struct NodeLOD {
    Cell type = Cell::EMPTY;
    NodeStep step = NodeStep::NONE;
};

struct NodeCellRow: NodeLOD {
    long originId = 0;
    int ext = 0;

    int size = 0;
    int score = 0;
    int tmpscr = 0;

    void incrementSize(bool cellIsDead) {
        ++size;
        if (cellIsDead || type == Cell::EMPTY) {
            tmpscr = 0;
        } else {
            if (score < ++tmpscr)
                score = tmpscr;
        }
    }

    NodeCellRow *prev = 0;
    NodeCellRow *next = 0;

    ~NodeCellRow() {}
};

template <typename Node> 
struct AdjacentNode {
    bool dead = false;

    Node* v[4] = { NULL, NULL, NULL, NULL }; // v[0]=right, v[1]=bottomRight, v[2]=bottom, v[3]=bottomLeft
    constexpr Node*& operator[](size_t i) { return v[i]; }
};

Cell Grid::getWinningLineColor() const {
    const unsigned d = boardDimension;
    const int dmax = d * d;

    auto iterateNode = [this, &dmax, &d](std::function<void(long, long, long)> &populateNode) {
        for (long id = 0; id < dmax; ++id) {
            const Vector2D cellPoint = Vector2D::createFromIndex(id, d);
            for (long ext = 0; ext < 4; ++ext) {
                const Vector2D newPoint = cellPoint + *(extptr + ext);
                if (!isInside(newPoint)) continue;
                const long nid = newPoint.toIndex(d);

                populateNode(id, nid, ext);
            }
        }
    };

    std::deque<NodeLOD> nodeLODsGarbage;
    std::vector<AdjacentNode<NodeLOD>> gridLOD;
    gridLOD.reserve(d * d);
    for (int id = 0; id < dmax; ++id)
        gridLOD.push_back({});

    std::function<NodeLOD*(Cell)> createLOD = [&nodeLODsGarbage](Cell type) {
        nodeLODsGarbage.push_back(NodeLOD{});
        NodeLOD *next =  &nodeLODsGarbage.back();
        next->type = type;
        next->step = NodeStep::LOOKING;
        return next;
    };

    std::function<void(long, long, long)> populateLOD =
    [this, &gridLOD, &createLOD](long id, long nid, long ext) {
        NodeLOD*& cell = gridLOD[id][ext];
        NodeLOD*& next = gridLOD[nid][ext];

        if (cell == NULL) { // COMPARE FIRST & SECOND PIECE

        } else if (cell->step == NodeStep::LOOKING) { // COMPARE SECOND & THIRD PIECE
            if (grid[id] == grid[nid]) {
                cell->step = NodeStep::BOILING;
                next = cell;
                return ;
            }
        } else if (cell->step == NodeStep::BOILING) { // COMPARE THIRD & FOURTH PIECE
            if (grid[id] != grid[nid] && cell->type != grid[nid]) {
                cell->step = NodeStep::DEATH;
            }
        }
        if (grid[nid] != Cell::EMPTY && grid[id] != grid[nid]) {
            next = createLOD(grid[id]);
        }
    };

    iterateNode(populateLOD);
    for (int id = 0; id < dmax; ++id) {
        for (long ext = 0; ext < 4; ++ext) {
            NodeLOD*& cellLOD = gridLOD[id][ext];
            if (cellLOD != NULL && cellLOD->step == NodeStep::DEATH) {
                gridLOD[id].dead = true;
                break ;
            }
        }
    }

    std::deque<NodeCellRow> cellRowsGarbage;
    std::vector<AdjacentNode<NodeCellRow>> gridCR;
    gridCR.reserve(d * d);
    for (int id = 0; id < dmax; ++id)
        gridCR.push_back({});

    std::function<NodeCellRow*()> createCell = [&cellRowsGarbage]() {
        cellRowsGarbage.push_back(NodeCellRow{});
        NodeCellRow *cell = &cellRowsGarbage.back();
        cell->step = NodeStep::LOOKING;
        return cell;
    };

    std::function<void(long, long, long)> populateCell =
    [this, &gridLOD, &gridCR, &createCell](long id, long nid, long ext) {
        NodeCellRow*& cell = gridCR[id][ext];
        NodeCellRow*& next = gridCR[nid][ext];

        // Init
        if (cell == NULL) { // cell->step == NodeStep::LOOKING
            cell = createCell();
        }
        if (cell->step == NodeStep::LOOKING) {
            cell->originId = id;
            cell->type = grid[id];
            cell->ext = ext;
            cell->step = NodeStep::BOILING;
        }
        // Increment, link and detect the end of the line 
        if (cell->step == NodeStep::BOILING) {
            cell->incrementSize(gridLOD[id].dead);

            if (grid[id] == grid[nid]) {
                next = cell; // Link the next grid cell to this Node
            } else { // End of the line
                next = createCell();
                // Link next & prev
                cell->next = next;
                next->prev = cell;
            }
        }
    };

    iterateNode(populateCell);

    for (const NodeCellRow& cellRows : cellRowsGarbage) {
        if (cellRows.score >= 5 && cellRows.type != Cell::EMPTY) {
            return cellRows.type;
        }
    }
    return Cell::EMPTY;
}

/**
 * @param apply If false just read, without modify member variable
 * @return number of pairs captured (1 for two stones captured)
 */
long Grid::handleCaptures(unsigned const id, bool const apply) {
    const Cell myColor = grid[id];
    if (myColor == Cell::EMPTY) return 0;
    const Cell enemyColor = (myColor == Cell::BLACK ? Cell::WHITE : Cell::BLACK);
    const unsigned d = boardDimension;

    long c = 0;
    const Vector2D cellPoint = Vector2D::createFromIndex(id, d);
    for (Vector2D offsetPoint : EXTREMITIES) {
        const Vector2D newPoint = cellPoint + offsetPoint * 3;
        if (!isInside(newPoint)) continue;
        const long nid = newPoint.toIndex(d);
        if (grid[nid] != myColor) continue;
        const long nid1 = (cellPoint + offsetPoint).toIndex(d);
        if (grid[nid1] != enemyColor) continue;
        const long nid2 = (cellPoint + offsetPoint * 2).toIndex(d);
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
    const unsigned d = boardDimension;

    const Vector2D cellPoint = Vector2D::createFromIndex(id, d);
    const Vector2D offsetPoint = *(extptr + ext + offset);
    do {
        const Vector2D newPoint = cellPoint + offsetPoint * (count + 1);
        if (!isInside(newPoint)) break;
        const long nid = newPoint.toIndex(d);
        bc = grid[nid];
        if (bc != myColor) break;
        if (alignedCells) alignedCells->insert(nid);
    } while (++count <= d);
    return count;
}

bool Grid::isDoubleThree(unsigned const id) const {
    const Cell myColor = grid[id];
    if (myColor == Cell::EMPTY) return false;

    long c = const_cast<Grid*>(this)->handleCaptures(id);
    if (c > 0) {
        std::cout << "It is a capture ! (" << c << ")" << std::endl;
        return false;
    }

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
        const bool bool1 = l + r >= 2 || specialThree;
        if (bool1) {
            std::cout << "[" << (myColor == Cell::BLACK ? "B" : "W") << " " << SPINNER[ext % 4] << "] ";
            std::cout << l << " + " << r << " + 1 = " << (l+r+1);
        }
        if (specialThree) {
            std::cout << " 3s: it is a special free-three." << std::endl;
            ++c;
        } else if (l + r >= 4) {
            std::cout << " 5: Probably a win." << std::endl;
            return false;
        } else if (l + r == 3 && lc == Cell::EMPTY && rc == Cell::EMPTY) {
            std::cout << " 4: Cannot be blocked !" << std::endl;
            return false;
        } else if (l + r == 3) {
            std::cout << " 4: Can be blocked :(" << std::endl;
            return false;
        } else if (l + r == 2 && lc == Cell::EMPTY && rc == Cell::EMPTY) {
            std::cout << " 3: it is a free-three." << std::endl;
            ++c;
        } else if (bool1) {
            std::cout << std::endl;
        }
    }

    if (c > 1)
        return true;
    return false;
}
            /* JE NE SAIS PAS SI CE CODE PEUT NOUS SERVIR POUR L'IA ???
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
