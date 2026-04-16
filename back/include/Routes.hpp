#pragma once
#include "Server.hpp"
#include "GameState.hpp"

void registerRoutes(Server& server, GameState& gs);
void registerRoutes_Arbiter(Server& server, GameState& gs);