#include <iostream>

#include "Server.hpp"
#include "Routes.hpp"

int	main(int ac, char **av)
{
	(void)ac;
	(void)av;
	std::cout << "hello Server!" << std::endl;

	Server server(9012);

	GameState gs;
	registerRoutes(server, gs);
	server.start();

	return(0);
}
