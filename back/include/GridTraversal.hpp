#pragma once
#include <deque>
#include <vector>

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

template <typename Node> 
struct AdjacentNode {
	bool dead = false;

	Node* v[4] = { NULL, NULL, NULL, NULL }; // v[0]=right, v[1]=bottomRight, v[2]=bottom, v[3]=bottomLeft
	constexpr Node*& operator[](size_t i) { return v[i]; }
};

class Grid;
class GridTraversal {
private:
	bool isLODGenerated = false;
	std::deque<NodeLOD> nodeLODsGarbage;
	std::vector<AdjacentNode<NodeLOD>> gridLOD;
	bool isCellRowGenerated = false;
	std::deque<NodeCellRow> cellRowsGarbage;
	std::vector<AdjacentNode<NodeCellRow>> gridCellRow;
	Grid &grid;

public:
	GridTraversal(Grid &grid);

	/*GridTraversal(const GridTraversal&) = default;
	GridTraversal &operator=(const GridTraversal &other) {
		if (this == &other)
			return *this;

		// grid is a reference: it cannot be rebound here.
		isLODGenerated = other.isLODGenerated;
		isCellRowGenerated = other.isCellRowGenerated;

		nodeLODsGarbage = other.nodeLODsGarbage;
		gridLOD = other.gridLOD;
		cellRowsGarbage = other.cellRowsGarbage;
		gridCellRow = other.gridCellRow;

		auto remapLODPtr = [&](NodeLOD *p) -> NodeLOD * {
			if (!p) return nullptr;
			for (size_t i = 0; i < other.nodeLODsGarbage.size(); ++i) {
				if (p == &other.nodeLODsGarbage[i])
					return &nodeLODsGarbage[i];
			}
			return nullptr;
		};

		auto remapCellRowPtr = [&](NodeCellRow *p) -> NodeCellRow * {
			if (!p) return nullptr;
			for (size_t i = 0; i < other.cellRowsGarbage.size(); ++i) {
				if (p == &other.cellRowsGarbage[i])
					return &cellRowsGarbage[i];
			}
			return nullptr;
		};

		for (auto &adj : gridLOD) {
			for (size_t d = 0; d < 4; ++d)
				adj[d] = remapLODPtr(adj[d]);
		}

		for (size_t i = 0; i < cellRowsGarbage.size(); ++i) {
			cellRowsGarbage[i].prev = remapCellRowPtr(other.cellRowsGarbage[i].prev);
			cellRowsGarbage[i].next = remapCellRowPtr(other.cellRowsGarbage[i].next);
		}

		for (auto &adj : gridCellRow) {
			for (size_t d = 0; d < 4; ++d)
				adj[d] = remapCellRowPtr(adj[d]);
		}

		return *this;
	}*/

	void clean();

	AdjacentNode<NodeCellRow> operator[](size_t i);

	void iterateNode(void (GridTraversal::*populateNode)(long, long, long));

	NodeLOD* createLOD(Cell type);
	void populateLOD(long id, long nid, long ext);
	void generateLOD();

	NodeCellRow* createCellRow();
	void populateCellRow(long id, long nid, long ext);
	void generateCellRow();

	void generate();

	 const std::deque<NodeLOD>& getNodeLODsGarbage();
	 const std::vector<AdjacentNode<NodeLOD>>& getGridLOD();
	 const std::deque<NodeCellRow>& getCellRowsGarbage();
	 const std::vector<AdjacentNode<NodeCellRow>>& getGridCellRow();
};
