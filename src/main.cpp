#include <iostream>

#include "Server.hpp"

int	main(int ac, char **av)
{
	(void)ac;
	(void)av;
	std::cout << "hello Server!" << std::endl;

    Server server(9012);

	GameState gs;
    server.start(gs);

	return(0);
}
