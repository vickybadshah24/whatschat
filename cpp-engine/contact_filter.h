#pragma once
#include <string>
#include <unordered_set>
#include <vector>

// Checks whether a WhatsApp sender ID is on the allowed list.
// If the list is empty every sender is allowed.
class ContactFilter {
public:
    explicit ContactFilter(const std::vector<std::string>& allowed);

    // Returns true if the sender should receive a reply.
    bool is_allowed(const std::string& sender_id) const;

    void add(const std::string& sender_id);
    void remove(const std::string& sender_id);

private:
    bool allow_all_ = true;
    std::unordered_set<std::string> allowed_;
};
