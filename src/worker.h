#pragma once
#include <QObject>
#include <QString>
#include "ollama.h"

class Worker : public QObject
{
    Q_OBJECT

public:
    enum class Action { Summarize = 0, Anki = 1, Both = 2 };

    explicit Worker(QObject* parent = nullptr);

public slots:
    void processUrl(const QString& url, int action);
    void processChat(const QString& message);

signals:
    // type: "status" | "assistant" | "summary" | "anki" | "error"
    void output(const QString& text, const QString& type);
    void done();

private:
    static std::string stripMarkdown(const std::string& text);

    ChatSession m_session;
};
