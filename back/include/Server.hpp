#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <ostream>

#include "GameState.hpp"


class Server {
public:
    typedef std::unordered_multimap<std::string, std::string> QueryMap;
    struct Request {
        std::string method;
        std::string path;
        QueryMap query;
        std::string body;
    };

    struct Response {
        int _status  = 200;
        std::string _body = "";
        std::string _content_type = "application/json";
        Response& status(int s) { _status = s; return *this; }
        Response& body(const std::string& b) { _body = b; return *this;}
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
    QueryMap parseQueryString(const std::string& request) const;
    std::string decodeURIComponent(std::string str) const;
    std::string readFullRequest(int client);
    std::string parseBody(const std::string& request) const;
    std::string buildResponse(const Response& res) const;
    void dispatch(int client, const Request& req);
};

inline std::ostream &operator<<(std::ostream &os, const Server::QueryMap &map) {
    bool first = true;
    for (const auto &[key, val] : map) {
        if (!first) os << " ";
        os << key << "=" << val;
        first = false;
    }
    return os;
}