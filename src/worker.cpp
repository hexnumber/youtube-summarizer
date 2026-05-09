#include "worker.h"
#include "urlhandler.h"
#include "summarize.h"
#include <sstream>
#include <fstream>
#include <cstdlib>

Worker::Worker(QObject* parent) : QObject(parent) {}

std::string Worker::stripMarkdown(const std::string& text)
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

void Worker::processChat(const QString& message)
{
    std::string response = sendMessage(m_session, message.toStdString());
    emit output(QString::fromStdString(response), "assistant");
    emit done();
}

void Worker::processUrl(const QString& url, int actionInt)
{
    Action action = static_cast<Action>(actionInt);
    std::string stdUrl = url.toStdString();

    if (!URLHandler::isSafeUrl(stdUrl))
    {
        emit output("URL contains invalid characters.", "error");
        emit done();
        return;
    }

    std::string videoId = URLHandler::extractVideoId(stdUrl);
    if (videoId.empty())
    {
        emit output("Could not extract video ID from URL.", "error");
        emit done();
        return;
    }

    emit output("Downloading subtitles (lang: " +
                QString::fromStdString(URLHandler::SUBTITLE_LANG) + ")...", "status");

    std::string cmd = "yt-dlp --write-auto-sub --sub-lang " + URLHandler::SUBTITLE_LANG +
                      " --skip-download \"" + stdUrl + "\" > /dev/null 2>&1";
    if (std::system(cmd.c_str()) != 0)
    {
        emit output("yt-dlp failed — is it installed?", "error");
        emit done();
        return;
    }

    std::string vttFile = URLHandler::findVttFile(videoId);
    if (vttFile.empty())
    {
        emit output("No subtitle file found for video: " +
                    QString::fromStdString(videoId), "error");
        emit done();
        return;
    }

    emit output("Cleaning transcript...", "status");
    std::string transcript = VTTCleaner::clean(vttFile);

    if (transcript.empty())
    {
        emit output("Transcript is empty after cleaning.", "error");
        emit done();
        return;
    }

    if (action == Action::Summarize || action == Action::Both)
    {
        emit output("Summarizing...", "status");
        std::string prompt =
            "Please summarize the following YouTube video transcript concisely, "
            "highlighting the main topics and key points discussed:\n\n" + transcript;
        std::string response = sendMessage(m_session, prompt);
        emit output(QString::fromStdString(response), "summary");
    }

    if (action == Action::Anki || action == Action::Both)
    {
        emit output("Generating Anki flashcards...", "status");
        std::string prompt =
            "Generate Anki flashcards from the following YouTube video transcript.\n\n"
            "Output ONLY the flashcards as tab-separated values, one card per line, "
            "with exactly two fields: question<TAB>answer. "
            "Do not include a header row, markdown formatting, code fences, or any other text. "
            "Keep questions concise and answers brief but complete.\n\n"
            "Transcript:\n" + transcript;

        std::string response = sendMessage(m_session, prompt);
        std::string cards = stripMarkdown(response);

        std::string filename = "anki_" + videoId + ".csv";
        std::ofstream file(filename);
        if (file.is_open())
        {
            file << "#separator:tab\n#html:false\n" << cards;
            file.close();
        }

        emit output(QString::fromStdString(cards) +
                    "\nSaved to: " + QString::fromStdString(filename), "anki");
    }

    emit done();
}
