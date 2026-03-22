#include "chat_history.h"

ChatHistory::ChatHistory(int max_per_contact)
    : max_per_contact_(max_per_contact * 2)  // user + assistant pairs
{}

std::vector<Message> ChatHistory::get(const std::string& sender_id) const {
    auto it = data_.find(sender_id);
    if (it == data_.end()) return {};
    return std::vector<Message>(it->second.begin(), it->second.end());
}

void ChatHistory::add(const std::string& sender_id,
                      const std::string& role,
                      const std::string& content)
{
    auto& q = data_[sender_id];
    q.push_back({role, content});
    while (static_cast<int>(q.size()) > max_per_contact_)
        q.pop_front();
}

void ChatHistory::clear(const std::string& sender_id) {
    data_.erase(sender_id);
}
