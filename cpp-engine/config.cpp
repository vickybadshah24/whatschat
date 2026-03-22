#include "config.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>
#include <cstdlib>

using json = nlohmann::json;

Config Config::load(const std::string& path) {
    Config cfg;

    // Defaults
    cfg.persona_name        = "Alex";
    cfg.persona_description =
        "You are Alex, a software developer. "
        "You are friendly and keep replies short and conversational. "
        "Never reveal you are an AI. "
        "Match the language (Hindi, English, Hinglish) the user writes in.";
    cfg.allowed_contacts         = {};
    cfg.max_history_per_contact  = 10;
    cfg.engine_port              = 8765;
    cfg.anthropic_api_key        = "";

    // Try to read API key from environment first
    const char* env_key = std::getenv("ANTHROPIC_API_KEY");
    if (env_key) cfg.anthropic_api_key = env_key;

    // Load JSON file
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "[Config] WARNING: config.json not found, using defaults.\n";
        return cfg;
    }

    try {
        json j;
        f >> j;

        if (j.contains("persona")) {
            auto& p = j["persona"];
            if (p.contains("name"))        cfg.persona_name        = p["name"];
            if (p.contains("description")) cfg.persona_description = p["description"];
        }

        if (j.contains("allowed_contacts")) {
            for (auto& c : j["allowed_contacts"])
                cfg.allowed_contacts.push_back(c.get<std::string>());
        }

        if (j.contains("max_history_per_contact"))
            cfg.max_history_per_contact = j["max_history_per_contact"];

        if (j.contains("engine_port"))
            cfg.engine_port = j["engine_port"];

        // JSON key takes priority only if env var not set
        if (cfg.anthropic_api_key.empty() && j.contains("anthropic_api_key"))
            cfg.anthropic_api_key = j["anthropic_api_key"];

    } catch (const std::exception& e) {
        std::cerr << "[Config] Parse error: " << e.what() << "\n";
    }

    return cfg;
}
