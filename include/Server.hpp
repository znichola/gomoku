#pragma once

#include <string>
#include <functional>
#include <unordered_map>

#include "GameState.hpp"


class Server {
public:
    struct Request {
        std::string method;
        std::string path;
        std::string query;
        std::string body;
    };

    struct Response {
        int status  = 200;
        std::string body = "";
        std::string content_type = "application/json";
    };

    using Handler = std::function<Response(const Request&)>;

    Server(int port);

    Server() = delete;
    Server(const Server&) = delete;
    Server(Server&&) = delete;
    Server& operator=(const Server&) = delete;
    Server& operator=(Server&&) = delete;

    ~Server();

    void start();
    void get(const std::string& path, Handler handler);

private:
    int server_fd;

    std::unordered_map<std::string, Handler> get_handlers;

    std::string parseMethod(const std::string &request) const;
    std::string parsePath(const std::string& request) const;
    std::string parseQueryString(const std::string& request) const;
    std::string readFullRequest(int client);
    std::string parseBody(const std::string& request) const;
    std::string buildResponse(const Response& res) const;
    void dispatch(int client, const Request& req);
};

