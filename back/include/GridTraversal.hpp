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

template <typename Node> 
struct AdjacentNode {
	bool dead = false;

	Node* v[4] = { NULL, NULL, NULL, NULL }; // v[0]=right, v[1]=bottomRight, v[2]=bottom, v[3]=bottomLeft
	constexpr Node*& operator[](size_t i) { return v[i]; }
	constexpr const Node* operator[](size_t i) const { return v[i]; }
};

class GridTraversal;
extern std::unordered_map<uint64_t, GridTraversal> tableGridTraversal;

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
	void clean();

	const AdjacentNode<NodeCellRow> operator[](const unsigned i) const;

	void iterateNode(void (GridTraversal::*populateNode)(long, long, long));

	NodeLOD* createLOD(Cell type);
	void populateLOD(long id, long nid, long ext);
	void generateLOD();

	NodeCellRow* createCellRow();
	void populateCellRow(long id, long nid, long ext);
	void generateCellRow();

	void generate();

	 const std::deque<NodeLOD>& getNodeLODsGarbage() const;
	 const std::vector<AdjacentNode<NodeLOD>>& getGridLOD() const;
	 const std::deque<NodeCellRow>& getCellRowsGarbage() const;
	 const std::vector<AdjacentNode<NodeCellRow>>& getGridCellRow() const;
};
