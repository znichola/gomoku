#pragma once

#include "Server.hpp"
#include "GameState.hpp"

// DEBUG FUNCTION FOR WATCHER
void handleLoadGameState(const Server::QueryMap& query, GameState& gs);

Cell parseCell(const std::string& s);
