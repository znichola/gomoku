#include "Routes.hpp"

using Request = Server::Request;
using Response = Server::Response;

void registerRoutes(Server& server, GameState& gs) {
    server.get("/gameState", [&gs](const Request& req) -> Response {
        (void)req;
        return Response{200, gs.serialize()};
    });

    server.get("/move", [&gs](const Request& req) -> Response {
        auto it = req.query.find("id");
        if (it == req.query.end())
            return Response{400, "missing 'id' query parameter"};
        unsigned id = std::stoi(it->second);
        if (!gs.playMove(id))
            return Response{400, "invalid move"};
        return Response{200, gs.serialize()};
    });
}
