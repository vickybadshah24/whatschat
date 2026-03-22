#pragma once
#include <string>
#include <vector>

struct Config {
    // AI persona
    std::string persona_name;
    std::string persona_description;

    // Whitelist (empty = allow all)
    std::vector<std::string> allowed_contacts;

    // Conversation memory
    int max_history_per_contact = 10;

    // Network
    int         engine_port      = 8765;
    std::string anthropic_api_key;

    // Load from JSON file (falls back to defaults if file missing)
    static Config load(const std::string& path);
};
