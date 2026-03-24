#include "Routes.hpp"

using Request = Server::Request;
using Response = Server::Response;

void registerRoutes(Server& server, GameState& gs) {
    server.get("/gameState", [&gs](const Request& req) -> Response {
        (void)req;
        Response res;
        res.body = gs.serialize();
        return res;
    });
 
    server.get("/move", [&gs](const Request& req) -> Response {
        Response res;

        unsigned id = std::stoi(req.query);
        
        gs.doMove(id);

        res.body = gs.serialize();

        return res;
    });
}
 