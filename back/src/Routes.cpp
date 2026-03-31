#include "Routes.hpp"
#include "Utils.hpp"
#include "MessageQueue.hpp"

#include <iostream>

using Request = Server::Request;
using Response = Server::Response;

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
        MessageQueue::drain(); // Draing before doing next move and refilling messages TODO: should be an API endpoint
        if (errno == ERANGE || !gs.playMove(id))
            return Response{400, "{\"error\": \"invalid move\"}"};
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
        else if (it->second == "erase") {
            Server::QueryMap::const_iterator it = req.query.find("id");
            if (it == req.query.end())
                return Response{400, "missing 'id' query parameter"};
            unsigned id = static_cast<unsigned>(std::stoul(it->second));
            gs.board.grid.setEmpty(id);
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
            } catch (const std::exception& e) {
                return Response{400, std::string("{\"error\":\"") + e.what() + "\"}"};
            }
        } else
            return Response{400, "{ \"error\": \"invalid action\" }"};
        return Response{200, gs.serialize()};
    });
}
