#include "claude_api.h"
#include "nlohmann/json.hpp"
#include <curl/curl.h>
#include <iostream>
#include <stdexcept>

using json = nlohmann::json;

// libcurl write callback
static size_t write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    std::string* buf = static_cast<std::string*>(userdata);
    buf->append(ptr, size * nmemb);
    return size * nmemb;
}

ClaudeAPI::ClaudeAPI(const std::string& api_key,
                     const std::string& system_prompt)
    : api_key_(api_key), system_prompt_(system_prompt)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

std::string ClaudeAPI::chat(const std::vector<Message>& messages) {
    if (api_key_.empty()) {
        std::cerr << "[Claude] ERROR: ANTHROPIC_API_KEY not set.\n";
        return "Sorry, I'm having trouble right now. I'll get back to you soon!";
    }

    // Build messages array
    json msgs_json = json::array();
    for (const auto& m : messages)
        msgs_json.push_back({{"role", m.role}, {"content", m.content}});

    json body = {
        {"model",      "claude-sonnet-4-20250514"},
        {"max_tokens", 300},
        {"system",     system_prompt_},
        {"messages",   msgs_json}
    };

    std::string response_body;
    try {
        response_body = post_json(
            "https://api.anthropic.com/v1/messages",
            api_key_,
            body.dump()
        );
    } catch (const std::exception& e) {
        std::cerr << "[Claude] HTTP error: " << e.what() << "\n";
        return "Sorry, couldn't reach AI right now!";
    }

    // Parse response
    try {
        auto resp = json::parse(response_body);
        if (resp.contains("error")) {
            std::cerr << "[Claude] API error: " << resp["error"]["message"] << "\n";
            return "Something went wrong. I'll check later!";
        }
        return resp["content"][0]["text"].get<std::string>();
    } catch (const std::exception& e) {
        std::cerr << "[Claude] Parse error: " << e.what() << "\n";
        return "I couldn't process that. Try again?";
    }
}

std::string ClaudeAPI::post_json(const std::string& url,
                                 const std::string& api_key,
                                 const std::string& body_str)
{
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("curl_easy_init failed");

    std::string response;

    curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("x-api-key: " + api_key).c_str());
    headers = curl_slist_append(headers, "anthropic-version: 2023-06-01");

    curl_easy_setopt(curl, CURLOPT_URL,            url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,     headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,     body_str.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,  (long)body_str.size());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,      &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,        30L);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
        throw std::runtime_error(curl_easy_strerror(res));

    return response;
}
