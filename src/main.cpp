#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

enum class Role {
    User,
    Assistant
};

struct Message {
    Role role;
    string content;
};

// convert role enum to the string ollama expects
string roleToString(Role role) {
    if (role == Role::User) return "user";
    return "assistant";
}

struct OllamaConfig {
    string host = "http://localhost:11434";
    string model = "nemotron-3-super:cloud";

    string chatUrl() const {
        return host + "/api/chat";
    }
};

struct ChatSession {
    OllamaConfig config;
    vector<Message> history;
};

size_t writeCallback(char* ptr, size_t size, size_t nmemb, string* response) {
    response->append(ptr, size * nmemb);
    return size * nmemb;
}

string sendMessage(ChatSession& session, const string& userInput) {
    session.history.push_back({ Role::User, userInput });

    // build messages array from history
    json messages = json::array();
    for (const Message& msg : session.history) {
        messages.push_back({
            {"role", roleToString(msg.role)},
            {"content", msg.content}
        });
    }

    json requestBody = {
        {"model", session.config.model},
        {"messages", messages},
        {"stream", false}
    };

    string requestStr = requestBody.dump();

    CURL* curl = curl_easy_init();
    string responseStr;

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

    if (result != CURLE_OK) {
        cerr << "curl error: " << curl_easy_strerror(result) << endl;
        return "";
    }

    json parsed = json::parse(responseStr);

    if (parsed.contains("error")) {
        cerr << "Ollama error: " << parsed["error"] << endl;
        return "";
    }

    string reply = parsed["message"]["content"];
    session.history.push_back({ Role::Assistant, reply });

    return reply;
}

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    ChatSession session;

    cout << "=== YouTube Summarizer - Chat ===" << endl;
    cout << "Model: " << session.config.model << endl;
    cout << "Type 'exit' to quit" << endl;
    cout << "---------------------------------" << endl << endl;

    string input;

    while (true) {
        cout << "You: ";
        getline(cin, input);

        if (input == "exit" || input == "quit") break;
        if (input.empty()) continue;

        cout << endl << "Assistant: " << flush;
        string response = sendMessage(session, input);
        cout << response << endl << endl;
    }

    curl_global_cleanup();
    cout << "Bye!" << endl;

    return 0;
}
