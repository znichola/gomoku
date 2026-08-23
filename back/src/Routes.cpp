#include "Routes.hpp"
#include "Utils.hpp"
#include "MessageQueue.hpp"
#include "EnumIO.hpp"

#include <iostream>

using Request = Server::Request;
using Response = Server::Response;

void registerRoutes(Server& server, GameState& gs) {
    registerRoutes_Arbiter(server, gs);

    server.get("/api/gameState", [&gs](const Request& req) -> Response {
        (void)req;
        return Response{200, gs.serialize()};
    });

    server.get("/api/move", [&gs](const Request& req) -> Response {
        Server::QueryMap::const_iterator it = req.query.find("id");
        if (it == req.query.end())
            return Response{400, "missing 'id' query parameter"};
        unsigned id = static_cast<unsigned>(std::stoul(it->second));

        it = req.query.find("force_color");
        if (it != req.query.end()) // Debug
            gs.board.isBlackToPlay = (it->second == "black");

        if (gs.board.winner != Cell::EMPTY) {
            MQ << "GAME ENDED !";
            MQ << "[AI] eval : " << AI::evaluate(gs.board, 0, gs.board.winner);
            return Response{200, gs.serialize()};
        }

        bool played = false;
        if (errno == ERANGE || !(played = gs.playMove(id)))
            MQ << "INVALID MOVE";
            // return Response{400, "{\"error\": \"invalid move\"}"};

        AI::evaluate(gs.board, 0, gs.board.winner);
        gs.pushMoveSuggestion();

        return Response{200, gs.serialize()};
    });

    server.get("/api/ai-move", [&gs](const Request& req) -> Response {
        (void)req; // no query params expected

        if (gs.board.winner != Cell::EMPTY)
            return Response{400, "{\"error\": \"game has already ended\"}"};

        const Cell toPlay = gs.board.isBlackToPlay ? Cell::BLACK : Cell::WHITE;
        if (gs.isAIGame != toPlay)
            return Response{400, "{\"error\": \"it is not the AI's move\"}"};

        MessageQueue::drain(); // Draing before doing next move and refilling messages

        const Cell aiPlayed = gs.askAI2Play();
        if (aiPlayed == Cell::OUTSIDE) {
            MQ << "INVALID MOVE FROM AI";
            return Response{400, "{\"error\": \"AI failed to produce a move\"}"};
        }
        gs.pushMoveSuggestion();

        return Response{200, gs.serialize()};
    });

    server.get("/api/reset", [&gs](const Request& req) -> Response {
        (void)req;
        gs.reset();
        MessageQueue::drain();
        gs.askAI2Play();
        gs.pushMoveSuggestion();
        return Response{200, gs.serialize()};
    });

    server.get("/api/set-config", [&gs](const Request& req) -> Response {
        if (!handleLoadAIState(req.query, gs)) {
            return Response{400, "{ \"error\": \"invalid action\" }"};
        }
        gs.pushMoveSuggestion();

        return Response{200, gs.serialize()};
    });

    server.get("/api/replay", [&gs](const Request& rq) -> Response {
        if (rq.query.find("moves") != rq.query.end()) {
            std::vector<std::string> m = splitCSV(rq.query.at("moves"));
            MessageQueue::drain();
            gs.reset();
            DISABLE_LOG
            for (const auto& s : m) {
                unsigned move = static_cast<unsigned>(std::stoul(s));
                gs.playMove(move);
            }
            gs.askAI2Play();
            ENABLE_LOG
            gs.pushMoveSuggestion();
        } else {
            return Response{400, "missing 'moves' query parameter"};
        }
        COUT << "Replaying to position: " << rq.query.at("moves") << "\n";
        return Response{200, gs.serialize()};
    });

    server.get("/api/debug-action", [&gs](const Request& req) -> Response {
        Server::QueryMap::const_iterator it = req.query.find("action");
        if (it == req.query.end())
            return Response{400, "missing 'action' query parameter"};
        std::string action = it->second;
        if (it->second == "make-double-tree")
            gs.makeDoubleTree();
        else if (it->second == "set-reset")
            gs.setReset();
        else if (it->second == "erase") {
            Server::QueryMap::const_iterator it = req.query.find("id");
            if (it == req.query.end())
                return Response{400, "missing 'id' query parameter"};
            unsigned id = static_cast<unsigned>(std::stoul(it->second));
            gs.board.grid.setEmpty(id);
            VictoryInfo info = gs.board.checkVictory();
            gs.board.winner = info.winner;
            gs.board.winByCaptures = info.byCaptures;
            gs.board.winByAlignment = info.byAlignment;
            gs.board.winningCells = info.winningCells;
        } else if (it->second == "minus") {
            Server::QueryMap::const_iterator it = req.query.find("id");
            if (it == req.query.end())
                return Response{400, "missing 'id' query parameter"};
            unsigned id = static_cast<unsigned>(std::stoul(it->second));
            if (id == static_cast<unsigned>(Cell::BLACK) && gs.board.blackCaptured > 0)
                gs.board.blackCaptured--;
            else if (id == static_cast<unsigned>(Cell::WHITE) && gs.board.whiteCaptured > 0)
                gs.board.whiteCaptured--;
        } else if (it->second == "load-game-state") {
            try {
                handleLoadGameState(req.query, gs);
                gs.pushMoveSuggestion();
            } catch (const std::exception& e) {
                return Response{400, std::string("{\"error\":\"") + e.what() + "\"}"};
            }
        } else
            return Response{400, "{ \"error\": \"invalid action\" }"};
        return Response{200, gs.serialize()};
    });

    // Same query parameters as /debug-action?action=load-game-state (board_grid, board_width,
    // board_blackCaptured, board_whiteCaptured, board_isBlackToPlay, moveHistory, plus optional
    // isAIGame/searchDepth/aiTimeBudgetMs/searchFunction/moveFunction to trigger a move), but loaded into a
    // throwaway GameState instead of the live one - lets a position be analyzed (what would the
    // AI play here, and why) without touching whatever game is actually in progress.
    server.get("/api/analyze", [](const Request& req) -> Response {
        GameState scratch;
        // MessageQueue is a global, shared with whatever live game is in progress - start clean
        // so this analysis doesn't pick up stray leftover messages, and drain again afterwards
        // so this analysis's own messages don't leak into the live game's next /gameState poll.
        MessageQueue::drain();

        // searchDepth/aiTimeBudgetMs/searchFunction/moveFunction all land on AI:: globals, not on
        // `scratch` - they're shared with whatever live game is in progress, same problem as
        // MessageQueue above but for engine config instead of messages: without saving and
        // restoring them here, an /analyze call with e.g. searchFunction=MINMAX or a shallow
        // searchDepth would silently degrade the live game's AI for every move after it, directly
        // contradicting this endpoint's "without touching whatever game is actually in progress"
        // contract above.
        const auto savedSearchFn  = AI::searchFunction;
        const auto savedMoveFn    = AI::moveFunction;
        const auto savedMaxDepth  = AI::maxDepth;
        const auto savedThinkMs   = AI::maxThinkMillis;

        Response res;
        try {
            handleLoadGameState(req.query, scratch);
            res = Response{200, scratch.serialize()};
        } catch (const std::exception& e) {
            res = Response{400, std::string("{\"error\":\"") + e.what() + "\"}"};
        }

        AI::searchFunction = savedSearchFn;
        AI::moveFunction   = savedMoveFn;
        AI::maxDepth       = savedMaxDepth;
        AI::maxThinkMillis = savedThinkMs;

        MessageQueue::drain();
        return res;
    });
}
