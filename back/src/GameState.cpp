#include <sstream>
#include <iostream>
#include <random>
#include <tuple>

#include "GameState.hpp"
#include "MessageQueue.hpp"
#include "AI.hpp"

std::string GameState::serialize() const {
    std::ostringstream out;

    out << "{\n";
    out << "\"isAIGame\": " << static_cast<unsigned>(isAIGame) << ",\n";
    out << "\"moveHistory\": [";
    for (size_t i = 0; i < moveHistory.size(); ++i) {
        out << moveHistory[i];
        if (i + 1 < moveHistory.size())
            out << ",";
    }
    out << "],\n\"board\": " << board.serialize() << ",\n";
    out << "\"messages\": [";
    for (size_t i = 0; i < MessageQueue::messages.size(); ++i) {
        out << "\"" << MessageQueue::messages[i] << "\"";
        if (i + 1 <  MessageQueue::messages.size())
            out << ",";
    }
    out << "]\n}";

    return out.str();
}

bool GameState::playMove(unsigned id) {
    bool res = false;
    COUT << "Playing move: " << id << std::endl;
    res = board.playMove(id);
    if (res) moveHistory.push_back(id);
    return res;
}

/**
 * @return Return false if error (IA sending a wrong move), true if IA not enabled or has played
 */
bool GameState::askAI2Play() {
    const Cell activePlayer = board.isBlackToPlay ? Cell::BLACK : Cell::WHITE;
    if (isAIGame == activePlayer) {
        MQ << "AI is thinking of a good move";
        DISABLE_LOG
        COUT << "[AI] ";
        auto res = playMove(AI::play(board, activePlayer == Cell::WHITE));
        ENABLE_LOG
        return res;
    }
    return true;
}

static Grid* rGrid = nullptr;

void GameState::reset() {
    board = Board(board.grid.width);
    if (rGrid != nullptr && moveHistory.size() >= 1)
        board.grid = *rGrid;
    moveHistory.clear();
}

void GameState::setReset() {
    if (rGrid == nullptr)
        rGrid = new Grid(board.grid);
    else
        *rGrid = board.grid;
}

static std::mt19937 gen(std::random_device{}());
static std::uniform_int_distribution<int> dist(0, 3);

bool GameState::makeDoubleTree() {
    auto &grid = board.grid;
    if (moveHistory.size() <= 0) return false;
    const unsigned d = grid.width;

    unsigned lastMove = moveHistory.back();
    COUT << "lastMove: " << lastMove << std::endl;
    Cell cellColor = grid[lastMove];
    const std::initializer_list<std::tuple<long, long, Cell>> cells = {
        {-1,  0, cellColor},
        { 0,  0, cellColor},
        { 1,  0, Cell::EMPTY},
        { 1,  1, Cell::EMPTY},
        { 3, -2, cellColor},
        { 4, -3, cellColor},
    };
    int rotation = dist(gen);
    auto rotatePoint = [](long x, long y, int rotations) -> Vector2D {
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
        grid.set(id, content);
    }
    return false;
}

void GameState::reload(const std::vector<Cell>& newGrid,
                    unsigned blackCaptured,
                    unsigned whiteCaptured,
                    bool isBlackToPlay,
                    unsigned width,
                    const std::vector<unsigned>& newMoveHistory,
                    Cell newisAIGame) {
    const size_t expected = static_cast<size_t>(width) * static_cast<size_t>(width);
    if (newGrid.size() != expected) {
        throw std::runtime_error("reload: board_grid size mismatch (expected " +
                                 std::to_string(expected) + ", got " +
                                 std::to_string(newGrid.size()) + ")");
    }

    isAIGame = newisAIGame;

    board.grid = Grid(width);
    board.grid.setGrid(newGrid);

    board.blackCaptured = blackCaptured;
    board.whiteCaptured = whiteCaptured;
    board.isBlackToPlay = isBlackToPlay;

    moveHistory = newMoveHistory;
}
