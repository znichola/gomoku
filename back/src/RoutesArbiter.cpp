#include "Routes.hpp"
#include "MessageQueue.hpp"
#include "Utils.hpp"

using Request = Server::Request;
using Response = Server::Response;

enum class AiState : unsigned {
	OPEN = 1 << 0,    // 0001: Gomoku reachable and able to communicate
	READY = 1 << 1,   // 0010: Ready to launch a game
	BUSY = 1 << 2     // 0100: In game or not (if not ready and not busy = unavailable)
};

constexpr unsigned operator|(AiState a, AiState b) {
  return static_cast<unsigned>(a) | static_cast<unsigned>(b);
}

constexpr unsigned operator&(unsigned a, AiState b) {
  return a & static_cast<unsigned>(b);
}

constexpr unsigned& operator|=(unsigned& a, AiState b) {
  a = a | static_cast<unsigned>(b);
  return a;
}

void registerRoutes_Arbiter(Server& server, GameState& gs) {
	static bool inGame = false;
	static std::string sessionid;

	server.get("/arbiter/ping", [&gs](const Request& req) -> Response {
		(void)req;
		(void)gs;
		std::ostringstream out;

		unsigned state = AiState::OPEN | AiState::READY;
		if (inGame) {
			state |= AiState::BUSY;
		}

		out << "{\n";
		out << "\"state\": " << state << ",\n";
		out << "\"sessionId\": " << ((inGame) ? ("\"" + sessionid + "\"") : "null") << "\n";
		out << "}";

		return Response{200, out.str()};
	});

	server.get("/arbiter/start", [&gs](const Request& req) -> Response {
		Server::QueryMap::const_iterator it = req.query.find("sessionid");
		if (it == req.query.end() || it->second.size() == 0)
			return Response{400, "missing 'sessionid' query parameter"};
		else if ((inGame || (sessionid.size() > 0 && sessionid != it->second))
					&& req.query.find("force") == req.query.end())
			return Response{403, "a game is already running."};
		sessionid = it->second;

		it = req.query.find("you_are");
		if (it == req.query.end())
			return Response{400, "missing 'you_are' query parameter"};
		std::string color = it->second;
		if (color == "black")
			gs.isAIGame = Cell::BLACK;
		else if (color == "white")
			gs.isAIGame = Cell::WHITE;
		else
			return Response{400, "wrong value for 'your_are'"};

    gs.reset();
		inGame = true;

		std::ostringstream out;

		out << "{\n";
		out << "\"done\": \"OK\"\n";
		out << "}";
		return Response{200, out.str()};
	});

	server.get("/arbiter/play", [&gs](const Request& req) -> Response {
		Server::QueryMap::const_iterator it = req.query.find("sessionid");
		if (it == req.query.end() || it->second.size() == 0)
			return Response{400, "missing 'sessionid' query parameter"};
		else if (!inGame)
			return Response{403, "no game is running"};
		else if (sessionid.size() == 0 || sessionid != it->second)
			return Response{403, "wrong 'sessionid' doesn't match."};

		it = req.query.find("gstatus");
		if (it == req.query.end())
			return Response{400, "missing 'gstatus' query parameter"};
		std::string gstatus = it->second;

		it = req.query.find("move");
		if (it == req.query.end()) {
			if (gstatus == "playing" && gs.moveHistory.size() != 0)
				return Response{400, "missing 'move' query parameter"};
		} else {
			unsigned mid = static_cast<unsigned>(std::stoul(it->second));
			if (!gs.playMove(mid))
				return Response{200, "{\"as_played\": false, \"because\": \"foe_wrongmove\", \"msglog\": \"wrong move\"}"};
			
			it = req.query.find("board");
			if (it == req.query.end())
				return Response{400, "missing 'board' query parameter"};
			std::vector<Cell> oppGrid = parseGridCSV(it->second);
			if (oppGrid != gs.board.grid.grid)
				return Response{200, "{\"as_played\": false, \"because\": \"board_doesntmatch\", \"msglog\": \"board state doesn't match\"}"};
		}
	
		unsigned nbMove = gs.moveHistory.size();
		if (!gs.askAI2Play())
			return Response{200, "{\"as_played\": false, \"because\": \"self_error\", \"msglog\": \"wrong move\"}"};

		if (gs.moveHistory.size() == nbMove)
			return Response{200, "{\"as_played\": false, \"because\": \"self_error\", \"msglog\": \"Unknow error, AI didn't play\"}"};

		std::ostringstream out;

		out << "{\n";
		out << "\"as_played\": true,\n";
		out << "\"move\": " << gs.moveHistory.back() << ",\n";
		out << "\"board\": " << gs.board.grid.serialize() << ",\n";
		out << "\"turn\": " << gs.moveHistory.size() << ",\n";
		out << "\"gstatus\": ";
    Cell victory = gs.board.isVictory();
		switch (victory) {
			case Cell::EMPTY: out << "\"playing\"\n"; break;
			case Cell::BLACK: out << "\"black_win\"\n"; break;
			case Cell::WHITE: out << "\"white_win\"\n"; break;
			case Cell::OUTSIDE: out << "\"draw\"\n"; break;
		}
		out << "}";
		return Response{200, out.str()};
	});
}
