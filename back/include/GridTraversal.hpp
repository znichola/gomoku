#pragma once
#include <deque>
#include <vector>
#include <unordered_map>

#include "Cell.hpp"
#include "Grid.hpp"
#include "Vector2D.hpp"

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

	// Length of the stone line
	int size = 0;
	// Score of living stone
	int score = 0;
	int _tmpscr = 0;

	void incrementSize(bool cellIsDead);

	NodeCellRow *prev = 0;
	NodeCellRow *next = 0;

	~NodeCellRow() {}
};

struct AdjacentNode {
	bool dead = false;

	NodeLOD* lod[4] = { NULL, NULL, NULL, NULL }; // v[0]=right, v[1]=bottomRight, v[2]=bottom, v[3]=bottomLeft
	NodeCellRow* cr[4] = { NULL, NULL, NULL, NULL }; // v[0]=right, v[1]=bottomRight, v[2]=bottom, v[3]=bottomLeft

	constexpr NodeCellRow*& operator[](size_t i) { return cr[i]; }
	constexpr const NodeCellRow* operator[](size_t i) const { return cr[i]; }
};

class GridTraversal;
extern std::unordered_map<uint64_t, GridTraversal> tableGridTraversal;

class Grid;
class GridTraversal {
private:
	std::deque<NodeLOD> nodeLODsGarbage;
	std::deque<NodeCellRow> cellRowsGarbage;
	std::vector<AdjacentNode> gridCellRow;

	void iterateNode(const Grid& grid, void (GridTraversal::*populateNode)(long, long, long, const Grid&));

	NodeLOD* createLOD(Cell type);
	void populateLOD(long id, long nid, long ext, const Grid& grid);

	NodeCellRow* createCellRow();
	void populateCellRow(long id, long nid, long ext, const Grid& grid);

public:
	GridTraversal(const Grid &grid);

	const AdjacentNode operator[](const unsigned i) const;

	const std::deque<NodeLOD>& getNodeLODsGarbage() const;
	const std::deque<NodeCellRow>& getCellRowsGarbage() const;
	const std::vector<AdjacentNode>& getGridCellRow() const;
};
