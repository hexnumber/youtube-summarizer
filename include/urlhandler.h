#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstdlib>
#include "summarize.h"

#include "ollama.h"

class URLHandler
{
public:
    inline static std::string SUBTITLE_LANG = "de";

    static bool isYouTubeUrl(const std::string& input)
    {
        return input.find("youtube.com") != std::string::npos ||
               input.find("youtu.be") != std::string::npos;
    }

    // reject URLs with shell-unsafe characters before passing to system()
    static bool isSafeUrl(const std::string& url)
    {
        for (char c : url)
        {
            if (c == '"' || c == '`' || c == '$' || c == '\\' || c == '\n' || c == '\r')
                return false;
        }
        return true;
    }

    static std::string extractVideoId(const std::string& url)
    {
        // youtu.be/VIDEO_ID
        size_t pos = url.find("youtu.be/");
        if (pos != std::string::npos)
        {
            std::string after = url.substr(pos + 9);
            for (char delim : {'?', '&', '#', '/'})
            {
                size_t p = after.find(delim);
                if (p != std::string::npos) after = after.substr(0, p);
            }
            return after;
        }

        // youtube.com/watch?v=VIDEO_ID
        pos = url.find("v=");
        if (pos != std::string::npos)
        {
            std::string after = url.substr(pos + 2);
            size_t p = after.find('&');
            if (p != std::string::npos) after = after.substr(0, p);
            return after;
        }

        return "";
    }

    static std::string findVttFile(const std::string& videoId)
    {
        std::string suffix = "." + SUBTITLE_LANG + ".vtt";
        for (const auto& entry : std::filesystem::directory_iterator("."))
        {
            if (!entry.is_regular_file()) continue;
            std::string name = entry.path().filename().string();
            if (name.find(videoId) != std::string::npos &&
                name.size() > suffix.size() &&
                name.substr(name.size() - suffix.size()) == suffix)
            {
                return entry.path().string();
            }
        }
        return "";
    }

    static void handle(ChatSession& session, const std::string& url)
    {
        if (!isSafeUrl(url))
        {
            std::cerr << "Error: URL contains invalid characters." << std::endl;
            return;
        }

        std::string videoId = extractVideoId(url);
        if (videoId.empty())
        {
            std::cerr << "Error: Could not extract video ID from URL." << std::endl;
            return;
        }

        std::cout << "Downloading subtitles (lang: " << SUBTITLE_LANG << ")..." << std::endl;

        std::string cmd = "yt-dlp --write-auto-sub --sub-lang " + SUBTITLE_LANG +
                          " --skip-download \"" + url + "\" > /dev/null 2>&1";
        int ret = std::system(cmd.c_str());
        if (ret != 0)
        {
            std::cerr << "Error: yt-dlp failed (exit code " << ret << "). Is yt-dlp installed?" << std::endl;
            return;
        }

        std::string vttFile = findVttFile(videoId);
        if (vttFile.empty())
        {
            std::cerr << "Error: No ." << SUBTITLE_LANG << ".vtt subtitle file found for video ID: " << videoId << std::endl;
            return;
        }

        std::cout << "Cleaning transcript..." << std::endl;
        std::string transcript = VTTCleaner::clean(vttFile);

        if (transcript.empty())
        {
            std::cerr << "Error: Transcript is empty after cleaning." << std::endl;
            return;
        }

        Action action = promptAction();

        if (action == Action::Summarize || action == Action::Both)
            summarize(session, transcript);

        if (action == Action::Anki || action == Action::Both)
            generateAnkiCards(session, transcript, videoId);
    }

private:
    enum class Action { Summarize, Anki, Both };

    static Action promptAction()
    {
        std::cout << std::endl << "What would you like to do?" << std::endl;
        std::cout << "  [1] Summarize" << std::endl;
        std::cout << "  [2] Anki flashcards" << std::endl;
        std::cout << "  [3] Both" << std::endl;
        std::cout << "Choice: " << std::flush;

        std::string input;
        std::getline(std::cin, input);
        std::cout << std::endl;

        if (input == "2") return Action::Anki;
        if (input == "3") return Action::Both;
        return Action::Summarize;
    }

    static void summarize(ChatSession& session, const std::string& transcript)
    {
        std::string prompt = "Please summarize the following YouTube video transcript concisely, "
                             "highlighting the main topics and key points discussed:\n\n" + transcript;

        std::cout << "Summary: " << std::flush;
        std::string response = sendMessage(session, prompt);
        std::cout << response << std::endl << std::endl;
    }

    // removes ```lang ... ``` fences that LLMs often wrap output in
    static std::string stripMarkdown(const std::string& text)
    {
        std::string result;
        std::istringstream stream(text);
        std::string line;
        bool inBlock = false;

        while (std::getline(stream, line))
        {
            if (line.rfind("```", 0) == 0) { inBlock = !inBlock; continue; }
            if (!inBlock) result += line + "\n";
        }

        return result;
    }

    static void generateAnkiCards(ChatSession& session, const std::string& transcript, const std::string& videoId)
    {
        std::string prompt =
            "Generate Anki flashcards from the following YouTube video transcript.\n\n"
            "Output ONLY the flashcards as tab-separated values, one card per line, "
            "with exactly two fields: question<TAB>answer. "
            "Do not include a header row, markdown formatting, code fences, or any other text. "
            "Keep questions concise and answers brief but complete.\n\n"
            "Transcript:\n" + transcript;

        std::cout << "Generating Anki flashcards..." << std::endl;
        std::string response = sendMessage(session, prompt);
        std::string cards = stripMarkdown(response);

        std::string filename = "anki_" + videoId + ".csv";
        std::ofstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Error: Could not write to " << filename << std::endl;
            return;
        }

        file << "#separator:tab\n";
        file << "#html:false\n";
        file << cards;
        file.close();

        std::cout << cards << std::endl;
        std::cout << "Anki flashcards saved to: " << filename << std::endl << std::endl;
    }
};
