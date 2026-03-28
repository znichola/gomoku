#include "Routes.hpp"

#include <iostream>

using Request = Server::Request;
using Response = Server::Response;

static void handleLoadGameState(const Server::QueryMap& q, GameState& gs);

void registerRoutes(Server& server, GameState& gs) {
    server.get("/gameState", [&gs](const Request& req) -> Response {
        (void)req;
        return Response{200, gs.serialize()};
    });

    server.get("/move", [&gs](const Request& req) -> Response {
        Server::QueryMap::const_iterator it = req.query.find("id");
        if (it == req.query.end())
            return Response{400, "missing 'id' query parameter"};
        unsigned id = static_cast<unsigned>(std::stoul(it->second));
        if (errno == ERANGE || !gs.playMove(id))
            return Response{400, "invalid move"};
        return Response{200, gs.serialize()};
    });

    server.get("/reset", [&gs](const Request& req) -> Response {
        (void)req;
        gs.reset();
        return Response{200, gs.serialize()};
    });

    server.get("/debug-action", [&gs](const Request& req) -> Response {
        Server::QueryMap::const_iterator it = req.query.find("action");
        if (it == req.query.end())
            return Response{400, "missing 'action' query parameter"};
        std::string action = it->second;
        if (it->second == "make-double-tree")
            gs.makeDoubleTree();
        else if (it->second == "set-reset")
            gs.setReset();
        else if (it->second == "load-game-state") {
            try {
                handleLoadGameState(req.query, gs);
            } catch (const std::exception& e) {
                return Response{400, std::string("{\"error\":\"") + e.what() + "\"}"};
            }
        } else
            return Response{400, "{ \"error\": \"invalid action\" }"};
        return Response{200, gs.serialize()};
    });
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

static void handleLoadGameState(const Server::QueryMap& query, GameState& gs) {
    const bool isHumanGame = parseBool(getQueryRequired(query, "isHumanGame"));
    const std::vector<unsigned> moveHistory = parseMoveHistoryCSV(getQueryRequired(query, "moveHistory"));
    const unsigned boardDimentions =
        static_cast<unsigned>(std::stoul(getQueryRequired(query, "board_boardDimentions")));
    const unsigned blackCaptured =
        static_cast<unsigned>(std::stoul(getQueryRequired(query, "board_blackCaptured")));
    const unsigned whiteCaptured =
        static_cast<unsigned>(std::stoul(getQueryRequired(query, "board_whiteCaptured")));
    const bool isBlackToPlay = parseBool(getQueryRequired(query, "board_isBlackToPlay"));
    const std::vector<Cell> grid = parseGridCSV(getQueryRequired(query, "board_grid"));

    gs.reload(grid, blackCaptured, whiteCaptured, isBlackToPlay, boardDimentions, moveHistory, isHumanGame);
}
