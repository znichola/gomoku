#include <iostream>

#include "Server.hpp"
#include "Routes.hpp"
#include "Utils.hpp"

#ifndef PORT
#define PORT 9012
#endif

int	main() {
	Server server(PORT);

	std::random_device rd;
	uint64_t seed = ((uint64_t)rd() << 32) ^ rd();

	init_zobrist(seed);

	GameState gs;
	registerRoutes(server, gs);
	server.start();

	return(0);
}
