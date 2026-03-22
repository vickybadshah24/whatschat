// ============================================================
//  WhatsApp AI Bot — C++ Engine
//  Listens on HTTP port 8765, receives messages from Node.js,
//  calls Claude API, returns AI reply.
// ============================================================

#include "config.h"
#include "contact_filter.h"
#include "chat_history.h"
#include "claude_api.h"
#include "http_server.h"
#include <iostream>
#include <string>
#include <csignal>

// Global flag for clean shutdown
volatile std::sig_atomic_t g_running = 1;

void signal_handler(int) {
    g_running = 0;
    std::cout << "\n[Engine] Shutting down...\n";
}

int main() {
    std::signal(SIGINT,  signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::cout << "================================================\n";
    std::cout << "  WhatsApp AI Bot — C++ Engine v1.0\n";
    std::cout << "================================================\n\n";

    // Load config
    Config config = Config::load("config.json");
    std::cout << "[Config] Persona: " << config.persona_name << "\n";
    std::cout << "[Config] Allowed contacts: "
              << (config.allowed_contacts.empty() ? "ALL" : std::to_string(config.allowed_contacts.size()))
              << "\n";

    // Initialize components
    ContactFilter filter(config.allowed_contacts);
    ChatHistory   history(config.max_history_per_contact);
    ClaudeAPI     claude(config.anthropic_api_key, config.persona_description);
    HttpServer    server(config.engine_port);

    std::cout << "[Engine] Listening on port " << config.engine_port << "...\n\n";

    // Main request loop
    server.on_request([&](const std::string& sender,
                          const std::string& message) -> std::string
    {
        // 1. Check whitelist
        if (!filter.is_allowed(sender)) {
            std::cout << "[Filter] Blocked: " << sender << "\n";
            return "__SKIP__";   // Node.js bridge will not send a reply
        }

        std::cout << "[IN]  " << sender << ": " << message << "\n";

        // 2. Build conversation history for this contact
        auto msgs = history.get(sender);
        msgs.push_back({"user", message});

        // 3. Call Claude AI
        std::string reply = claude.chat(msgs);

        // 4. Update history
        history.add(sender, "user",      message);
        history.add(sender, "assistant", reply);

        std::cout << "[OUT] " << sender << ": " << reply << "\n\n";
        return reply;
    });

    // Run until Ctrl+C
    while (g_running) {
        server.poll(100);   // 100 ms tick
    }

    return 0;
}
