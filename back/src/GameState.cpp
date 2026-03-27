#include <sstream>
#include <iostream>
#include <random>

#include "GameState.hpp"

std::string GameState::serialize() const {
    std::ostringstream out;

    out << "{\n";
    out << "\"isHumanGame\": " << (isHumanGame ? "true" : "false") << ",\n";
    out << "\"moveHistory\": [";
    for (size_t i = 0; i < moveHistory.size(); ++i) {
        out << moveHistory[i];
        if (i + 1 < moveHistory.size())
            out << ",";
    }
    out << "],\n\"board\": " << board.serialize() << "\n";
    out << "}";

    return out.str();
}

bool GameState::playMove(unsigned id) {
    std::cout << "Playing move: " << id << "\n";
    moveHistory.push_back(id);
    return board.playMove(id);
}

void GameState::reset() {
    board = Board(board.grid.boardDimentions);
    moveHistory.clear();
}

static std::mt19937 gen(std::random_device{}());
static std::uniform_int_distribution<int> dist(0, 3);

bool GameState::makeDoubleTree() {
    auto &grid = board.grid;
    if (moveHistory.size() <= 0) return false;
    const unsigned d = grid.boardDimentions;

    unsigned lastMove = moveHistory.back();
    std::cout << "lastMove: " << lastMove << "\n";
    const std::initializer_list<std::tuple<long, long, Cell>> cells = {
        {-1,  0, Cell::WHITE},
        { 0,  0, Cell::WHITE},
        { 1,  0, Cell::EMPTY},
        { 1,  1, Cell::EMPTY},
        { 3, -2, Cell::WHITE},
        { 4, -3, Cell::WHITE},
    };
    int rotation = dist(gen);
    auto rotatePoint = [](long x, long y, int rotations) -> std::pair<long, long> {
        for (int i = 0; i < rotations; ++i) {
            auto temp = x;
            x = -y;
            y = temp;
        }
        return {x, y};
    };
    const long cx = lastMove % d;
    const long cy = lastMove / d;
    for (const auto& [x, y, content] : cells) {
        auto [rx, ry] = rotatePoint(x, y, rotation);
        const long nx = cx + rx;
        const long ny = cy + ry;
        const long id = ny * d + nx;
        if (!(0 <= nx && nx < d && 0 <= ny && ny < d)) continue;
        if (content == Cell::WHITE)
            grid.setWhite(id);
        else if (content == Cell::EMPTY)
            grid.setEmpty(id);
    }
    return false;
}
