#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>
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

    auto start_time = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(start_time);
    start_at = std::ctime(&time);
    if (start_at.back() == '\n') start_at.pop_back();
    std::cout << "Server started at " << start_at << std::endl;

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
 
            if (req.query.find("silent") == req.query.end()) {
                std::cout << req.method << " " << req.path << std::endl;
                if (req.query.size() > 0)
                    std::cout << "  query: " << req.query << std::endl;
            }
 
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
    if (req.method == "OPTIONS") {
        // CORS HANDLER
        res.status(204);
    }
    else if (handler)
        try {
            res = (*handler)(req);
        } catch (const std::exception& e) {
            res.status(500).body(std::string("{\"error\":\"") + e.what() + "\"}");
            std::cerr << "[500] " << req.method << " " << req.path << " — " << e.what() << "\n";
        } catch (...) {
            res.status(500).body("{\"error\":\"unknown internal error\"}");
            std::cerr << "[500] " << req.method << " " << req.path << " — unknown exception\n";
        }
    else {
        res.status(404).body("{\"error\":\"not found\"}");
        std::cerr << "[404] Unknown route " << req.path << "\n";
    }

    std::string raw = buildResponse(res);
    if (send(client, raw.c_str(), raw.size(), 0) < 0)
        perror("send");
}

std::string Server::buildResponse(const Response& res) const {
    std::ostringstream oss;
    std::string reason = (res._status == 200) ? "OK"
                       : (res._status == 204) ? "No Content"
                       : (res._status == 400) ? "Bad Request"
                       : (res._status == 404) ? "Not Found"
                       : "Internal Server Error";
 
    auto start_time = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(start_time);

    std::string time_str = std::ctime(&time);
    if (time_str.back() == '\n') time_str.pop_back();
    
    oss << "HTTP/1.1 " << res._status << " " << reason << "\r\n"
        << "Access-Control-Allow-Origin: *\r\n"
        << "Access-Control-Allow-Methods: *\r\n"
        << "Access-Control-Allow-Headers: *\r\n"
        << "Access-Control-Max-Age: 86400\r\n"
        << "Connection: close\r\n"
        << "Content-Length: " << res._body.size() << "\r\n"
        << "Content-Type: " << res._content_type << "\r\n"
        << "Date: " << time_str << "\r\n"
        << "Expires: " << start_at << "\r\n" // HACK for CORS reading header (https://stackoverflow.com/a/44816592)
        << "\r\n"
        << res._body;

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

Server::QueryMap Server::parseQueryString(const std::string &request) const {
    auto findQueryStringStartStop = [&request]() -> std::pair<size_t, size_t> {
        size_t stopFirstLine = request.find("\r\n");
        size_t start = request.find('?');
        if (start == std::string::npos || start > stopFirstLine)
            return {0, 0};
        start += 1;
        size_t stop = request.find(" ", start);
        return {start, stop};
    };

    auto [start, stop] = findQueryStringStartStop();
    QueryMap data;
    std::string key;
    std::string val;
    bool hasEquals = false;

    for (size_t i = start; i < stop; i++) {
        char c = request[i];
        if (c == '&' || c == '\0') {
            if (!key.empty())
                data.insert({decodeURIComponent(key), decodeURIComponent(val)});
            key.clear();
            val.clear();
            hasEquals = false;
        } else if (c == '=' && !hasEquals) {
            hasEquals = true;
        } else if (hasEquals) {
            val += c;
        } else {
            key += c;
        }
    }

    // Flush the last key/value pair
    if (!key.empty())
        data.insert({decodeURIComponent(key), decodeURIComponent(val)});

    return data;
}

std::string Server::parseBody(const std::string& request) const {
    size_t pos = request.find("\r\n\r\n");
    if (pos == std::string::npos) return "";
    return request.substr(pos + 4);
}

std::string Server::decodeURIComponent(std::string str) const {
    std::string::iterator it;
    std::string::iterator start;
    int mode = 0;
    int num;
    char repl[2] = {0};

    for (it = str.begin(); it != str.end(); it++) {
        if (*it == '%') {
            start = it;
            mode = 1;
        }
        else if (isxdigit(*it) && 1 <= mode && mode < 3) {
            mode++;
            if (mode == 3) {
                num = strtol(std::string(start + 1, it + 1).c_str(), NULL, 16);
                if (32 <= num && num <= 126) {
                    *repl = static_cast<char>(num);
                    str.replace(start, it + 1, repl);
                    it -= 2;
                }
                else if (0 <= num && num < 32) {
                    // Securité on accepte pas les caractères invisibles
                    str.erase(start, it + 1);
                    it -= 3;
                }
                mode = 0;
            }
        }
        else
            mode = 0;
    }
    return (str);
}

std::ostream &operator<<(std::ostream &os, const Server::QueryMap &map) {
    bool first = true;
    for (const auto &[key, val] : map) {
        if (!first) os << " ";
        if (val.length() <= 0)
            os << key;
        else if (key == "board_grid" && val.length() > 21)
            os << "board_grid=" << val.substr(0, 20) << "…";
        else
            os << key << "=" << val;
        first = false;
    }
    return os;
}
