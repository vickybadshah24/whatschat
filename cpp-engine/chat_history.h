#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <deque>

struct Message {
    std::string role;     // "user" or "assistant"
    std::string content;
};

// Stores per-contact conversation history with a rolling window.
class ChatHistory {
public:
    explicit ChatHistory(int max_per_contact = 10);

    // Get all messages for a contact (oldest first)
    std::vector<Message> get(const std::string& sender_id) const;

    // Append a message (automatically trims if over limit)
    void add(const std::string& sender_id,
             const std::string& role,
             const std::string& content);

    // Wipe history for a contact
    void clear(const std::string& sender_id);

private:
    int max_per_contact_;
    std::unordered_map<std::string, std::deque<Message>> data_;
};
