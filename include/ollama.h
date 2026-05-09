#pragma once
#include <string>
#include <vector>

enum class Role { User, Assistant };

struct Message
{
    Role role;
    std::string content;
};

struct OllamaConfig
{
    std::string host  = "http://localhost:11434";
    std::string model = "nemotron-3-super:cloud";

    std::string chatUrl() const { return host + "/api/chat"; }
};

struct ChatSession
{
    OllamaConfig config;
    std::vector<Message> history;
};

std::string sendMessage(ChatSession& session, const std::string& userInput);
