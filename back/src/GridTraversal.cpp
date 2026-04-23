#include "GridTraversal.hpp"

#include <iostream>

std::unordered_map<uint64_t, GridTraversal> tableGridTraversal;

void NodeCellRow::incrementSize(bool cellIsDead){
	++size;
	if (cellIsDead || type == Cell::EMPTY) {
		_tmpscr = 0;
	} else {
		if (score < ++_tmpscr)
			score = _tmpscr;
	}
}

GridTraversal::GridTraversal(Grid &grid): grid(grid) {

}

void GridTraversal::clean() {
	if (isLODGenerated) {
		nodeLODsGarbage.clear();
		gridLOD.clear();
		isLODGenerated = false;
	}
	if (isCellRowGenerated) {
		cellRowsGarbage.clear();
		gridCellRow.clear();
		isCellRowGenerated = false;
	}
}

const AdjacentNode<NodeCellRow> GridTraversal::operator[](unsigned i) const {
	const_cast<GridTraversal *>(this)->generate();
	return gridCellRow[i];
}

void GridTraversal::iterateNode(void (GridTraversal::*populateNode)(long, long, long)) {
	const unsigned width = grid.width;
	const unsigned size  = grid.size;

	for (long id = 0; id < size; ++id) {
		const Vector2D cellPoint = Vector2D::createFromIndex(id, width);
		for (long ext = 0; ext < 4; ++ext) {
			const Vector2D newPoint = cellPoint + *(extptr + ext);
			if (!grid.isInside(newPoint)) continue;
			const long nid = newPoint.toIndex(width);

			(this->*populateNode)(id, nid, ext);
		}
	}
};

NodeLOD* GridTraversal::createLOD(Cell type) {
	nodeLODsGarbage.push_back(NodeLOD{});
	NodeLOD *next = &nodeLODsGarbage.back();
	next->type = type;
	next->step = NodeStep::LOOKING;

	return next;
}

void GridTraversal::populateLOD(long id, long nid, long ext) {
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
}

void GridTraversal::generateLOD() {
	if (isLODGenerated)
		return ;
	const unsigned size = grid.size;

	gridLOD.reserve(size);
	gridLOD.insert(gridLOD.end(), size, AdjacentNode<NodeLOD>{});

	iterateNode(&GridTraversal::populateLOD);
	isLODGenerated = true;
}

NodeCellRow* GridTraversal::createCellRow() {
	cellRowsGarbage.push_back(NodeCellRow{});
	NodeCellRow *cell = &cellRowsGarbage.back();
	cell->step = NodeStep::LOOKING;
	return cell;
};

void GridTraversal::populateCellRow(long id, long nid, long ext) {
	NodeCellRow*& cell = gridCellRow[id][ext];
	NodeCellRow*& next = gridCellRow[nid][ext];

	// Init
	if (cell == NULL) { // cell->step == NodeStep::LOOKING
		cell = createCellRow();
	}
	if (cell->step == NodeStep::LOOKING) {
		cell->originId = id;
		cell->type = grid[id];
		cell->ext = ext;
		cell->step = NodeStep::BOILING;
	}
	// Increment, link and detect the end of the line 
	if (cell->step == NodeStep::BOILING) {
		cell->incrementSize(gridCellRow[id].dead);

		if (grid[id] == grid[nid]) {
			next = cell; // Link the next grid cell to this Node
		} else { // End of the line
			next = createCellRow();
			// Link next & prev
			cell->next = next;
			next->prev = cell;
		}
	}
};

void GridTraversal::generateCellRow() {
	if (isCellRowGenerated)
		return ;

	generateLOD();

	const unsigned size = grid.size;

	gridCellRow.reserve(size);
	gridCellRow.insert(gridCellRow.end(), size, AdjacentNode<NodeCellRow>{});

	// Set dead state
	for (unsigned id = 0; id < size; ++id) {
		for (long ext = 0; ext < 4; ++ext) {
			NodeLOD*& cellLOD = gridLOD[id][ext];
			if (cellLOD != NULL && cellLOD->step == NodeStep::DEATH) {
				// std::cout << id << " is dead" << std::endl;
				gridCellRow[id].dead = true;
				break ;
			}
		}
	}

	iterateNode(&GridTraversal::populateCellRow);
	isCellRowGenerated = true;
}

void GridTraversal::generate() {
	generateCellRow();
}

const std::deque<NodeLOD>& GridTraversal::getNodeLODsGarbage() const {
	const_cast<GridTraversal*>(this)->generateLOD();
	return nodeLODsGarbage;
};

const std::vector<AdjacentNode<NodeLOD>>& GridTraversal::getGridLOD() const {
	const_cast<GridTraversal*>(this)->generateLOD();
	return gridLOD;
};

const std::deque<NodeCellRow>& GridTraversal::getCellRowsGarbage() const {
	const_cast<GridTraversal*>(this)->generateCellRow();
	return cellRowsGarbage;
};

const std::vector<AdjacentNode<NodeCellRow>>& GridTraversal::getGridCellRow() const {
	const_cast<GridTraversal*>(this)->generateCellRow();
	return gridCellRow;
};
