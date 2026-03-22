#pragma once
#include "chat_history.h"
#include <string>
#include <vector>

// Calls the Anthropic Claude API (claude-sonnet) and returns a reply string.
class ClaudeAPI {
public:
    ClaudeAPI(const std::string& api_key,
              const std::string& system_prompt);

    // Send conversation history and get next assistant reply.
    // Returns empty string on error.
    std::string chat(const std::vector<Message>& messages);

private:
    std::string api_key_;
    std::string system_prompt_;

    static std::string post_json(const std::string& url,
                                 const std::string& api_key,
                                 const std::string& body);
};
