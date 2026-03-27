#include "Routes.hpp"

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
        unsigned id = std::stoul(it->second);
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
        else
            return Response{400, "invalid action"};
        return Response{200, gs.serialize()};
    });
}
