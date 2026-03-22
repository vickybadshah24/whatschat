#include "contact_filter.h"

ContactFilter::ContactFilter(const std::vector<std::string>& allowed) {
    if (!allowed.empty()) {
        allow_all_ = false;
        for (const auto& c : allowed)
            allowed_.insert(c);
    }
}

bool ContactFilter::is_allowed(const std::string& sender_id) const {
    if (allow_all_) return true;
    return allowed_.count(sender_id) > 0;
}

void ContactFilter::add(const std::string& sender_id) {
    allow_all_ = false;
    allowed_.insert(sender_id);
}

void ContactFilter::remove(const std::string& sender_id) {
    allowed_.erase(sender_id);
}
