#pragma once
#include <string>
#include <fstream>
#include <vector>
#include <regex>
#include <unordered_set>

class VTTCleaner
{
public:
    static bool isIgnorable(const std::string& line)
    {
        return line.find("WEBVTT") != std::string::npos ||
               line.find("Kind:") != std::string::npos ||
               line.find("Language:") != std::string::npos ||
               line.empty();
    }

    static bool isTimestampLine(const std::string& line)
    {
        return line.find("-->") != std::string::npos;
    }

    static std::string removeTags(const std::string& input)
    {
        std::regex tagPattern("<[^>]*>");
        return std::regex_replace(input, tagPattern, "");
    }

    static std::string removeDuplicates(const std::vector<std::string>& lines)
    {
        std::unordered_set<std::string> seen;
        std::string result;

        for (const auto& line : lines)
        {
            if (seen.find(line) == seen.end())
            {
                seen.insert(line);
                result += line + " ";
            }
        }

        return result;
    }

    static std::string clean(const std::string& filename)
    {
        std::ifstream file(filename);
        std::string line;
        std::vector<std::string> cleanedLines;

        while (std::getline(file, line))
        {
            if (isIgnorable(line))
                continue;
            if (isTimestampLine(line))
                continue;

            std::string cleaned = removeTags(line);
            cleaned = normalizeSpaces(cleaned);

            if (!cleaned.empty())
                cleanedLines.push_back(cleaned);
        }

        return removeDuplicates(cleanedLines);
    }

private:
    static std::string normalizeSpaces(const std::string& str)
    {
        std::string result;
        bool lastWasSpace = false;

        for (char c : str)
        {
            if (c == ' ')
            {
                if (!lastWasSpace)
                    result += c;
                lastWasSpace = true;
            }
            else
            {
                result += c;
                lastWasSpace = false;
            }
        }

        return result;
    }
};
