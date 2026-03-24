#include "Routes.hpp"

using Request = Server::Request;
using Response = Server::Response;

void registerRoutes(Server& server, GameState& gs) {
    server.get("/gameState", [&gs](const Request& req) -> Response {
        (void)req;
        Response res;
        return res.body(gs.serialize());
    });

    server.get("/move", [&gs](const Request& req) -> Response {
        Response res;

        auto it = req.query.find("id");
        if (it == req.query.end())
            return Response{400, "missing 'id' query parameter"};
        unsigned id = std::stoi(it->second);
        
        gs.doMove(id);

        return res.body(gs.serialize());
    });
}
