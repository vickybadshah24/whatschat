#include "http_server.h"
#include "nlohmann/json.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>

using json = nlohmann::json;

HttpServer::HttpServer(int port) : port_(port) {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0)
        throw std::runtime_error("socket() failed");

    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1 only
    addr.sin_port        = htons(port_);

    if (bind(server_fd_, (sockaddr*)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("bind() failed on port " + std::to_string(port_));

    if (listen(server_fd_, 16) < 0)
        throw std::runtime_error("listen() failed");

    // Non-blocking
    int flags = fcntl(server_fd_, F_GETFL, 0);
    fcntl(server_fd_, F_SETFL, flags | O_NONBLOCK);
}

HttpServer::~HttpServer() {
    if (server_fd_ >= 0) close(server_fd_);
}

void HttpServer::on_request(Handler handler) {
    handler_ = std::move(handler);
}

void HttpServer::poll(int timeout_ms) {
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(server_fd_, &rfds);

    timeval tv{};
    tv.tv_sec  = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int ret = select(server_fd_ + 1, &rfds, nullptr, nullptr, &tv);
    if (ret > 0 && FD_ISSET(server_fd_, &rfds))
        accept_and_handle();
}

void HttpServer::accept_and_handle() {
    sockaddr_in cli{};
    socklen_t len = sizeof(cli);
    int client_fd = accept(server_fd_, (sockaddr*)&cli, &len);
    if (client_fd < 0) return;

    std::string raw = read_request(client_fd);

    // Find JSON body (after blank line)
    auto pos = raw.find("\r\n\r\n");
    if (pos == std::string::npos) {
        close(client_fd);
        return;
    }
    std::string body = raw.substr(pos + 4);

    std::string reply_text;
    try {
        auto req   = json::parse(body);
        auto sender  = req.value("sender",  "");
        auto message = req.value("message", "");
        reply_text = handler_ ? handler_(sender, message) : "__SKIP__";
    } catch (...) {
        reply_text = "__ERROR__";
    }

    send_response(client_fd, reply_text);
    close(client_fd);
}

std::string HttpServer::read_request(int fd) {
    std::string buf;
    buf.reserve(4096);
    char chunk[1024];
    while (true) {
        ssize_t n = recv(fd, chunk, sizeof(chunk), 0);
        if (n <= 0) break;
        buf.append(chunk, n);
        // Stop once we have the full HTTP request (headers + body)
        if (buf.find("\r\n\r\n") != std::string::npos &&
            buf.size() > buf.find("\r\n\r\n") + 4)
            break;
    }
    return buf;
}

void HttpServer::send_response(int fd, const std::string& reply_text) {
    json resp = {{"reply", reply_text}};
    std::string body = resp.dump();

    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n"
        << "Content-Type: application/json\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Connection: close\r\n"
        << "\r\n"
        << body;

    std::string response = oss.str();
    send(fd, response.c_str(), response.size(), 0);
}
