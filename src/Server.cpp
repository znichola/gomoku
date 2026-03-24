#include <arpa/inet.h>
#include <unistd.h>
#include "iostream"
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

void Server::start(GameState gs) {
    (void)gs;
    std::signal(SIGINT, handleSigint);

    while(true) {
        int client = accept(server_fd, nullptr, nullptr);
        if (client < 0) {
            perror("accept");
            continue;
        }
        std::string request = readFullRequest(client);
        if (request.length() > 0) {
            std::string method = parseMethod(request);
            std::string path = parsePath(request);
            std::string queryString = parseQueryString(request);

            std::cout   << "\nREQUEST\n"
                        << "method: {" << method << "}\n"
                        << "path: {" << path << "}\n"
                        << "queryStrings: {"<< queryString << "}\n"
                        // << "requst {\n" << request 
                        ;
            std::string res = "HTTP/1.1 OK\r\nContent-Type: text/html;\r\nConnection: Close\r\nContent-Length: 6\r\n\r\nGOMOKU";
            if (send(client, res.c_str(), res.length(), 0) < 0) {
                perror("send");
            }
        }
        close(client);
    }
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

