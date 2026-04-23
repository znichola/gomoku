#include "GridTraversal.hpp"

#include <iostream>

std::unordered_map<uint64_t, GridTraversal> tableGridTraversal;

GridTraversal::GridTraversal(const Grid &grid) {
	const unsigned size = grid.size;

	// NodeLOD
	gridLOD.reserve(size);
	gridLOD.insert(gridLOD.end(), size, AdjacentNode<NodeLOD>{});

	iterateNode(grid, &GridTraversal::populateLOD);

	// NodeCellRow
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

	iterateNode(grid, &GridTraversal::populateCellRow);
}

const AdjacentNode<NodeCellRow> GridTraversal::operator[](unsigned i) const {
	return gridCellRow[i];
}

void GridTraversal::iterateNode(const Grid& grid, void (GridTraversal::*populateNode)(long, long, long, const Grid&)) {
	const unsigned width = grid.width;
	const unsigned size  = grid.size;

	for (long id = 0; id < size; ++id) {
		const Vector2D cellPoint = Vector2D::createFromIndex(id, width);
		for (long ext = 0; ext < 4; ++ext) {
			const Vector2D newPoint = cellPoint + *(extptr + ext);
			if (!grid.isInside(newPoint)) continue;
			const long nid = newPoint.toIndex(width);

			(this->*populateNode)(id, nid, ext, grid);
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

void GridTraversal::populateLOD(long id, long nid, long ext, const Grid& grid) {
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

NodeCellRow* GridTraversal::createCellRow() {
	cellRowsGarbage.push_back(NodeCellRow{});
	NodeCellRow *cell = &cellRowsGarbage.back();
	cell->step = NodeStep::LOOKING;
	return cell;
};

void GridTraversal::populateCellRow(long id, long nid, long ext, const Grid& grid) {
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

const std::deque<NodeLOD>& GridTraversal::getNodeLODsGarbage() const {
	return nodeLODsGarbage;
};

const std::vector<AdjacentNode<NodeLOD>>& GridTraversal::getGridLOD() const {
	return gridLOD;
};

const std::deque<NodeCellRow>& GridTraversal::getCellRowsGarbage() const {
	return cellRowsGarbage;
};

const std::vector<AdjacentNode<NodeCellRow>>& GridTraversal::getGridCellRow() const {
	return gridCellRow;
};

void NodeCellRow::incrementSize(bool cellIsDead){
	++size;
	if (cellIsDead || type == Cell::EMPTY) {
		_tmpscr = 0;
	} else {
		if (score < ++_tmpscr)
			score = _tmpscr;
	}
}
