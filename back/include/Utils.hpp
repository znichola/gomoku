#pragma once

#include "Server.hpp"
#include "GameState.hpp"

Cell parseCell(const std::string& s);
std::vector<Cell> parseGridCSV(const std::string& s);

// DEBUG FUNCTION FOR WATCHER
void handleLoadGameState(const Server::QueryMap& query, GameState& gs);
