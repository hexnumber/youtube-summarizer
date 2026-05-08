#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include "json.hpp"
#include "urlhandler.h"

using json = nlohmann::json;

enum class Role
{
    User,
    Assistant
};

struct Message
{
    Role role;
    std::string content;
};

// convert role enum to the string ollama expects
std::string roleToString(Role role)
{
    if (role == Role::User) return "user";
    return "assistant";
}

struct OllamaConfig
{
    std::string host = "http://localhost:11434";
    std::string model = "nemotron-3-super:cloud";

    std::string chatUrl() const
    {
        return host + "/api/chat";
    }
};

struct ChatSession
{
    OllamaConfig config;
    std::vector<Message> history;
};

size_t writeCallback(char* ptr, size_t size, size_t nmemb, std::string* response)
{
    response->append(ptr, size * nmemb);
    return size * nmemb;
}

std::string sendMessage(ChatSession& session, const std::string& userInput)
{
    session.history.push_back({ Role::User, userInput });

    // build messages array from history
    json messages = json::array();
    for (const Message& msg : session.history)
    {
        messages.push_back(
        {
            {"role", roleToString(msg.role)},
            {"content", msg.content}
        });
    }

    json requestBody =
    {
        {"model", session.config.model},
        {"messages", messages},
        {"stream", false}
    };

    std::string requestStr = requestBody.dump();

    CURL* curl = curl_easy_init();
    std::string responseStr;

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, session.config.chatUrl().c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestStr.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);

    CURLcode result = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (result != CURLE_OK)
    {
        std::cerr << "curl error: " << curl_easy_strerror(result) << std::endl;
        return "";
    }

    json parsed = json::parse(responseStr);

    if (parsed.contains("error"))
    {
        std::cerr << "Ollama error: " << parsed["error"] << std::endl;
        return "";
    }

    std::string reply = parsed["message"]["content"];
    session.history.push_back({ Role::Assistant, reply });

    return reply;
}

int main()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);

    ChatSession session;

    std::cout << "=== YouTube Summarizer - Chat ===" << std::endl;
    std::cout << "Model: " << session.config.model << std::endl;
    std::cout << "Paste a YouTube URL to summarize, or chat directly." << std::endl;
    std::cout << "Type 'exit' to quit" << std::endl;
    std::cout << "---------------------------------" << std::endl << std::endl;

    std::string input;

    while (true)
    {
        std::cout << "You: ";
        std::getline(std::cin, input);

        if (input == "exit" || input == "quit") break;
        if (input.empty()) continue;

        if (URLHandler::isYouTubeUrl(input))
        {
            URLHandler::handle(session, input);
            continue;
        }

        std::cout << std::endl << "Assistant: " << std::flush;
        std::string response = sendMessage(session, input);
        std::cout << response << std::endl << std::endl;
    }

    curl_global_cleanup();
    std::cout << "Bye!" << std::endl;

    return 0;
}
