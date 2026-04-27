#include "AI.jeteste.hpp"
#include <map>
#include <iostream>
#include <algorithm>

static constexpr float INF = std::numeric_limits<float>::infinity();
// static constexpr float WIN = 100000.0F; // Then endgame in x is maxDepth - depth

/*
MinMax algo - minimizing possible losses
https://en.wikipedia.org/wiki/Minimax#Pseudocode

maximisingPlayer is a flag to toggle searching for
lowest score or highest score possible.
black likes low scores, so it should be called with a false here.

TODO see negamax for simpler version of the function, and alpha-beta pruning for more optimised one
*/
/**
* @param maximizingPlayer (color == -1.0f ? Cell::BLACK : Cell::WHITE)
*/
float AI::minMax_jeteste1(const Board &board, int16_t depth, bool isBlackToPlay, int16_t level) {
	nodeVisitCounter[depth] += 1;
	if (depth == 0 || board.winner != Cell::EMPTY) {
		return evaluate(board, depth, board.winner); // evaluation/heuristic is only run for terminal nodes
	}
	
	float best = isBlackToPlay ? -INF : INF;
	//auto minmax = (isBlackToPlay) ? std::max<float> : std::min<float>;
	
	for (auto move : getCandidateMoves_jeteste1(board.grid, isBlackToPlay ? Cell::BLACK : Cell::WHITE, level)) {
		if (!board.isValidMove(move)) continue; // skip illegal moves
		float score = minMax_jeteste1(Board(board.grid, move), depth-1, !isBlackToPlay, level + 1);
		best = (isBlackToPlay) ? std::max(score, best) : std::min(score, best);
	}
	return best;
}

std::set<unsigned> AI::getCandidateMoves_jeteste1(const Grid &grid, Cell color, int16_t level) {
	const GridTraversal &gt = grid.nodes();
	const std::deque<NodeCellRow> &CellRows = gt.getCellRowsGarbage();

	std::map<unsigned, uint32_t> moves;

	(void) color;
	for (auto node = CellRows.begin(); node != CellRows.end(); ++node) {
		const NodeCellRow &cr = *node;
		if (cr.type != Cell::EMPTY) {
			const Vector2D origin = Vector2D::createFromIndex(cr.originId, grid.width);
			Vector2D vec = origin - EXTREMITIES[cr.ext];
			if (grid.isInside(vec)) {
				unsigned id = vec.toIndex(grid.width);
				moves[id] = ((moves.count(id) > 0) ? moves[id] : 0) + cr.score + cr.size;
			}
			vec = origin + EXTREMITIES[cr.ext] * cr.size;
			if (grid.isInside(vec)) {
				unsigned id = vec.toIndex(grid.width);
				moves[id] = ((moves.count(id) > 0) ? moves[id] : 0) + cr.score + cr.size;
			}
    } else if (cr.size == 1 
						&& cr.type == Cell::EMPTY
						&& cr.prev && cr.next
						&& (cr.next->type == Cell::BLACK || cr.next->type == Cell::WHITE)
						&& cr.next->type == cr.prev->type
						&& (cr.next->size + cr.prev->size) >= 3) {
			moves[cr.originId] = ((moves.count(cr.originId) > 0) ? moves[cr.originId] : 0)
				+ (cr.prev && cr.prev->type == color ? cr.prev->score + cr.prev->size + 10 : 0)
				+ (cr.next && cr.next->type == color ? cr.next->score + cr.next->size + 10 : 0);
		}
	}
	// Convertir la map en vecteur
	std::vector<std::pair<unsigned, uint32_t>> sortedMoves(moves.begin(), moves.end());

	// Trier seulement les 10 premiers
	size_t count = std::min(size_t(std::max(2, (10 - level * 2))), sortedMoves.size());
	std::partial_sort(sortedMoves.begin(), sortedMoves.begin() + count, sortedMoves.end(),
		[](const auto& a, const auto& b) {
			return a.second > b.second;  // Ordre décroissant
		});

	std::set<unsigned> result;

	for (size_t i = 0; i < count; ++i) {
		const auto& [id, score] = sortedMoves[i];
		result.insert(id);
	}

	return result;
}