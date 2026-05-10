#include "ollama.h"
#include "json.hpp"
#include <curl/curl.h>
#include <string>

using json = nlohmann::json;

static size_t writeCallback(char* ptr, size_t size, size_t nmemb, std::string* response)
{
    response->append(ptr, size * nmemb);
    return size * nmemb;
}

static std::string roleToString(Role role)
{
    return role == Role::User ? "user" : "assistant";
}

std::string sendMessage(ChatSession& session, const std::string& userInput)
{
    session.history.push_back({ Role::User, userInput });

    json messages = json::array();
    for (const Message& msg : session.history)
    {
        messages.push_back({
            {"role", roleToString(msg.role)},
            {"content", msg.content}
        });
    }

    json requestBody = {
        {"model",    session.config.model},
        {"messages", messages},
        {"stream",   false}
    };

    std::string requestStr = requestBody.dump();
    CURL* curl = curl_easy_init();
    std::string responseStr;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL,           session.config.chatUrl().c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,     requestStr.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,     headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,      &responseStr);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,        120L);

    CURLcode result = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (result != CURLE_OK)
        return "curl error: " + std::string(curl_easy_strerror(result));

    json parsed = json::parse(responseStr, nullptr, false);
    if (parsed.is_discarded() || parsed.contains("error"))
        return "Ollama error: " + responseStr;

    std::string reply = parsed["message"]["content"];
    session.history.push_back({ Role::Assistant, reply });
    return reply;
}
