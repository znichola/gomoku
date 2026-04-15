#include <iostream>

#include "Server.hpp"
#include "Routes.hpp"

#ifndef PORT
#define PORT 9012
#endif

int	main(int ac, char **av)
{
	(void)ac;
	(void)av;

	Server server(PORT);

	GameState gs;
	registerRoutes(server, gs);
	server.start();

	return(0);
}
