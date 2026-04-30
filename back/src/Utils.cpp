#include "Utils.hpp"
#include <EnumIO.hpp>
#include <MessageQueue.hpp>

uint64_t zob[361][3];

/**
 * Init Zobrist Hashing
 * https://www.chessprogramming.org/Zobrist_Hashing
 */
void init_zobrist(uint64_t seed = 0xF2D3FAF467E04098) {
    std::mt19937_64 rng(seed);

    for (int i = 0; i < 361; i++) {
        for (int v = 0; v < 3; v++) {
            zob[i][v] = rng();
        }
    }
}

Cell parseCell(const std::string& s) {
    if (s == "0") return Cell::EMPTY;
    if (s == "1") return Cell::BLACK;
    if (s == "2") return Cell::WHITE;
    throw std::runtime_error("invalid Cell: " + s);
}

std::vector<std::string> splitCSV(const std::string& s) {
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

std::vector<Cell> parseGridCSV(const std::string& s) {
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

bool handleLoadAIState(const Server::QueryMap& query, GameState& gs) {
    bool changed = false;

    auto has = [&query](const std::string& key) -> bool {
        return query.find(key) != query.end(); };

    auto get = [&query](const std::string& key) -> const std::string& {
        return query.at(key); };

    if (has("moveSuggestion")) {
        changed = true;
        gs.moveSuggestion = parseEnum<Cell>(get("moveSuggestion"), {
            {"off", Cell::EMPTY},
            {"black", Cell::BLACK},
            {"white", Cell::WHITE},
            {"both", Cell::OUTSIDE},
        });
    }

    if (has("searchFunction")) {
        changed = true;
        AI::searchFunction = parseEnum<AI::SearchFunction>(get("searchFunction"), {
            {"MINMAX", AI::SearchFunction::MINMAX},
            {"MINMAX_JETESTE", AI::SearchFunction::MINMAX_JETESTE},
            {"NEGAMAX", AI::SearchFunction::NEGAMAX},
            {"ALPHABETA_NEGAMAX", AI::SearchFunction::ALPHABETA_NEGAMAX},
            {"ALPHABETA_NEGAMAX_TT", AI::SearchFunction::ALPHABETA_NEGAMAX_TT}
        });
    }

    if (has("moveFunction")) {
        changed = true;
        AI::moveFunction = parseEnum<AI::MoveFunction>(get("moveFunction"), {
            {"CANDIDATE_MOVES", AI::MoveFunction::CANDIDATE_MOVES},
            {"CANDIDATE_MOVES_2", AI::MoveFunction::CANDIDATE_MOVES_2},
            {"JETEST", AI::MoveFunction::JETEST},
        });
    }

    if (has("searchDepth")) {
        changed = true;
        AI::maxDepth = std::stoi(get("searchDepth"));
    }

    if (has("isAIGame")) {
        changed = true;
        gs.isAIGame = parseCell(get("isAIGame"));
        MessageQueue::drain();
        gs.askAI2Play();
    }
    return changed;
}

void handleLoadGameState(const Server::QueryMap& query, GameState& gs) {
    const std::vector<unsigned> moveHistory = parseMoveHistoryCSV(getQueryRequired(query, "moveHistory"));
    const unsigned width =
        static_cast<unsigned>(std::stoul(getQueryRequired(query, "board_width")));
    const unsigned blackCaptured =
        static_cast<unsigned>(std::stoul(getQueryRequired(query, "board_blackCaptured")));
    const unsigned whiteCaptured =
        static_cast<unsigned>(std::stoul(getQueryRequired(query, "board_whiteCaptured")));
    const bool isBlackToPlay = parseBool(getQueryRequired(query, "board_isBlackToPlay"));
    const std::vector<Cell> grid = parseGridCSV(getQueryRequired(query, "board_grid"));

    gs.reload(grid, blackCaptured, whiteCaptured, isBlackToPlay, width, moveHistory);
    handleLoadAIState(query, gs);
}
