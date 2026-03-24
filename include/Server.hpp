#pragma once

#include <string>

#include "GameState.hpp"


class Server {
private:
    int server_fd;

    std::string parseMethod(const std::string &request) const;
    std::string parsePath(const std::string& request) const;
    std::string parseQueryString(const std::string& request) const;
    std::string readFullRequest(int client);

public:
    Server(int port);

    Server() = delete;
    Server(const Server&) = delete;
    Server(Server&&) = delete;
    Server& operator=(const Server&) = delete;
    Server& operator=(Server&&) = delete;

    ~Server();

    void start(GameState gs);
};

