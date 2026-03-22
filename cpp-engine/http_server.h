#pragma once
#include <string>
#include <functional>

// A minimal single-threaded HTTP server that listens for POST /message
// requests from the Node.js WhatsApp bridge.
//
// Request JSON:  { "sender": "919876543210@c.us", "message": "Hello!" }
// Response JSON: { "reply": "Hey, what's up?" }
//                { "reply": "__SKIP__" }   // bot should not reply

class HttpServer {
public:
    using Handler = std::function<std::string(
        const std::string& sender,
        const std::string& message)>;

    explicit HttpServer(int port);
    ~HttpServer();

    // Register the callback that processes each message
    void on_request(Handler handler);

    // Process pending connections for up to timeout_ms milliseconds
    void poll(int timeout_ms);

private:
    int         port_;
    int         server_fd_ = -1;
    Handler     handler_;

    void        accept_and_handle();
    std::string read_request(int client_fd);
    void        send_response(int client_fd, const std::string& reply_text);
};
