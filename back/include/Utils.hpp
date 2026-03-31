#pragma once

#include "Server.hpp"
#include "GameState.hpp"

// DEBUG FUNCTION FOR WATCHER
void handleLoadGameState(const Server::QueryMap& query, GameState& gs);

bool parseBool(const std::string& s);
