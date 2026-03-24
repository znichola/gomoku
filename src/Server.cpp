#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include "csignal"

#include "Server.hpp"

void handleSigint(int) {
    std::cout << "\nShutting down...\n";
    exit(0);
}

Server::~Server() {
    if (server_fd != -1 && close(server_fd) != 0) perror("destructor close");
}

Server::Server(int port) {

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); exit(1); }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt"); exit(1); }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {perror("bind"); exit(1); }
    if (listen(server_fd, 10) < 0) { perror("listen"); exit(1); }

    std::cout << "Server listening on port " << port << "...\n";
}

void Server::start() {
    std::signal(SIGINT, handleSigint);
 
    while (true) {
        int client = accept(server_fd, nullptr, nullptr);
        if (client < 0) { perror("accept"); continue; }
 
        std::string raw = readFullRequest(client);
        if (!raw.empty()) {
            Request req;
            req.method = parseMethod(raw);
            req.path   = parsePath(raw);
            req.query  = parseQueryString(raw);
            req.body   = parseBody(raw);
 
            std::cout << "\nREQUEST"
                      << "\n  method: " << req.method
                      << "\n  path:   " << req.path
                      << "\n  query:  " << req.query
                      << "\n";
 
            dispatch(client, req);
        }
        close(client);
    }
}

void Server::get(const std::string& path, Handler handler) {
    get_handlers[path] = handler;
}


void Server::dispatch(int client, const Request& req) {
    Handler* handler = nullptr;

    if (req.method == "GET" && get_handlers.count(req.path))
        handler = &get_handlers[req.path];

    Response res;
    if (handler)
        try {
            res = (*handler)(req);
        } catch (const std::exception& e) {
            res.status = 500;
            res.body   = std::string("{\"error\":\"") + e.what() + "\"}";
            std::cerr << "[500] " << req.method << " " << req.path << " — " << e.what() << "\n";

            // TODO remove when done
            throw e;
        } catch (...) {
            res.status = 500;
            res.body   = "{\"error\":\"unknown internal error\"}";
            std::cerr << "[500] " << req.method << " " << req.path << " — unknown exception\n";
        }
    else {
        res.status = 404;
        res.body   = "{\"error\":\"not found\"}";
        std::cerr << "[404] Unknown route " << req.path << "\n";
    }

    std::string raw = buildResponse(res);
    if (send(client, raw.c_str(), raw.size(), 0) < 0)
        perror("send");
}

std::string Server::buildResponse(const Response& res) const {
    std::ostringstream oss;
    std::string reason = (res.status == 200) ? "OK"
                       : (res.status == 400) ? "Bad Request"
                       : (res.status == 404) ? "Not Found"
                       : "Internal Server Error";
 
    oss << "HTTP/1.1 " << res.status << " " << reason << "\r\n"
        << "Content-Type: " << res.content_type << "\r\n"
        << "Connection: close\r\n"
        << "Content-Length: " << res.body.size() << "\r\n"
        << "\r\n"
        << res.body;
 
    return oss.str();
}

std::string Server::readFullRequest(int client) {
    std::string request;
    char buffer[4096];
    bool method_checked = false;

    while (true) {
        int bytes = read(client, buffer, sizeof(buffer));
        if (bytes <= 0) break;
        request.append(buffer, bytes);

        if (method_checked == false && bytes > 3) {
            std::string method = parseMethod(request);
            if (method != "GET") break;
        }

        if (request.find("\r\n\r\n") != std::string::npos)
            break;
    }

    return request;
}

std::string Server::parseMethod(const std::string& request) const {
    size_t end = request.find(" ");
    return request.substr(0, end);
}

std::string Server::parsePath(const std::string& request) const {
    size_t start = request.find(' ') + 1;
    size_t end = request.find_first_of(" ?", start);
    return request.substr(start, end - start);
}

std::string Server::parseQueryString(const std::string &request) const {
    size_t endFirstLine = request.find("\r\n");
    size_t start = request.find('?');
    if (start == std::string::npos || start > endFirstLine)
        return "";
    start += 1;
    size_t end = request.find(" ", start);
    return request.substr(start, end - start);
}

std::string Server::parseBody(const std::string& request) const {
    size_t pos = request.find("\r\n\r\n");
    if (pos == std::string::npos) return "";
    return request.substr(pos + 4);
}