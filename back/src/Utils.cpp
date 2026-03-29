#include "Utils.hpp"


// DEBUG FUNCTION FOR WATCHER

static bool parseBool(const std::string& s) {
    if (s == "true" || s == "1") return true;
    if (s == "false" || s == "0") return false;
    throw std::runtime_error("invalid boolean: " + s);
}

static std::string getQueryRequired(const Server::QueryMap& q, const std::string& key) {
    auto it = q.find(key);
    if (it == q.end()) throw std::runtime_error("missing query parameter: " + key);
    return it->second;
}

static std::vector<std::string> splitCSV(const std::string& s) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == ',') {
            out.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    out.push_back(cur);
    return out;
}

static std::vector<Cell> parseGridCSV(const std::string& s) {
    // s like: "0,0,2,1,..."
    // empty string => empty vector (but in your case should not be empty)
    std::vector<Cell> grid;
    if (s.empty()) return grid;

    auto parts = splitCSV(s);
    grid.reserve(parts.size());
    for (const auto& p : parts) {
        if (p.empty()) { grid.push_back(Cell::EMPTY); continue; }
        int v = std::stoi(p);
        if (v == 0) grid.push_back(Cell::EMPTY);
        else if (v == 1) grid.push_back(Cell::BLACK);
        else if (v == 2) grid.push_back(Cell::WHITE);
        else throw std::runtime_error("invalid cell value in board_grid: " + p);
    }
    return grid;
}

static std::vector<unsigned> parseMoveHistoryCSV(const std::string& s) {
    // moveHistory can be empty => []
    std::vector<unsigned> out;
    if (s.empty()) return out;

    auto parts = splitCSV(s);
    out.reserve(parts.size());
    for (const auto& p : parts) {
        if (p.empty()) continue;
        unsigned v = static_cast<unsigned>(std::stoul(p));
        out.push_back(v);
    }
    return out;
}

void handleLoadGameState(const Server::QueryMap& query, GameState& gs) {
    const bool isHumanGame = parseBool(getQueryRequired(query, "isHumanGame"));
    const std::vector<unsigned> moveHistory = parseMoveHistoryCSV(getQueryRequired(query, "moveHistory"));
    const unsigned boardDimension =
        static_cast<unsigned>(std::stoul(getQueryRequired(query, "board_boardDimension")));
    const unsigned blackCaptured =
        static_cast<unsigned>(std::stoul(getQueryRequired(query, "board_blackCaptured")));
    const unsigned whiteCaptured =
        static_cast<unsigned>(std::stoul(getQueryRequired(query, "board_whiteCaptured")));
    const bool isBlackToPlay = parseBool(getQueryRequired(query, "board_isBlackToPlay"));
    const std::vector<Cell> grid = parseGridCSV(getQueryRequired(query, "board_grid"));

    gs.reload(grid, blackCaptured, whiteCaptured, isBlackToPlay, boardDimension, moveHistory, isHumanGame);
}
