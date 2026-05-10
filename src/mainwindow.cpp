#include "mainwindow.h"
#include "worker.h"
#include "urlhandler.h"

#include <QApplication>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollBar>
#include <QThread>
#include <QStackedWidget>

// ── Stylesheets ───────────────────────────────────────────────────────────────

static const char* DARK_QSS = R"(
    QMainWindow                   { background: #1e1e1e; }
    QMainWindow > QWidget         { background: #1e1e1e; }
    QWidget#header                { background: #252526; border-bottom: 1px solid #3e3e42; }
    QWidget#actionBar             { background: #2d2d30; border-top:    1px solid #3e3e42; }
    QWidget#inputBar              { background: #252526; border-top:    1px solid #3e3e42; }
    QWidget#settingsContent       { background: #1e1e1e; }
    QLabel                        { background: transparent; color: #cccccc; }
    QLabel#title                  { font-size: 15px; font-weight: bold; }
    QLabel#actionLabel            { color: #888888; font-size: 13px; }
    QTextEdit#output              { background: #1e1e1e; color: #cccccc;
                                    border: none; font-size: 13px; padding: 4px; }
    QLineEdit#input               { background: #3c3c3c; color: #cccccc;
                                    border: 1px solid #555555; border-radius: 6px;
                                    padding: 0 12px; font-size: 13px; }
    QLineEdit#input:focus         { border-color: #007acc; }
    QPushButton#sendBtn           { background: #007acc; color: white;
                                    border: none; border-radius: 6px;
                                    font-size: 13px; font-weight: bold; }
    QPushButton#sendBtn:hover     { background: #1a8ad4; }
    QPushButton#sendBtn:pressed   { background: #005a9e; }
    QPushButton#sendBtn:disabled  { background: #4a4a4a; color: #777777; }
    QPushButton#themeBtn,
    QPushButton#settingsBtn,
    QPushButton#backBtn           { background: transparent; color: #cccccc;
                                    border: 1px solid #555555; border-radius: 6px;
                                    padding: 4px 10px; font-size: 13px; }
    QPushButton#themeBtn:hover,
    QPushButton#settingsBtn:hover,
    QPushButton#backBtn:hover     { background: #3c3c3c; }
    QRadioButton                  { background: transparent; color: #cccccc; font-size: 13px; }
    QRadioButton::indicator       { width: 14px; height: 14px; }
    QScrollBar:vertical           { background: #1e1e1e; width: 8px; margin: 0; }
    QScrollBar::handle:vertical   { background: #555555; border-radius: 4px; min-height: 20px; }
    QScrollBar::add-line:vertical,
    QScrollBar::sub-line:vertical { height: 0; }
)";

static const char* LIGHT_QSS = R"(
    QMainWindow                   { background: #f5f5f5; }
    QMainWindow > QWidget         { background: #f5f5f5; }
    QWidget#header                { background: #ffffff; border-bottom: 1px solid #e0e0e0; }
    QWidget#actionBar             { background: #f0f0f0; border-top:    1px solid #e0e0e0; }
    QWidget#inputBar              { background: #ffffff; border-top:    1px solid #e0e0e0; }
    QWidget#settingsContent       { background: #f5f5f5; }
    QLabel                        { background: transparent; color: #1e1e1e; }
    QLabel#title                  { font-size: 15px; font-weight: bold; }
    QLabel#actionLabel            { color: #666666; font-size: 13px; }
    QTextEdit#output              { background: #ffffff; color: #1e1e1e;
                                    border: none; font-size: 13px; padding: 4px; }
    QLineEdit#input               { background: #ffffff; color: #1e1e1e;
                                    border: 1px solid #cccccc; border-radius: 6px;
                                    padding: 0 12px; font-size: 13px; }
    QLineEdit#input:focus         { border-color: #0078d4; }
    QPushButton#sendBtn           { background: #0078d4; color: white;
                                    border: none; border-radius: 6px;
                                    font-size: 13px; font-weight: bold; }
    QPushButton#sendBtn:hover     { background: #106ebe; }
    QPushButton#sendBtn:pressed   { background: #005a9e; }
    QPushButton#sendBtn:disabled  { background: #cccccc; color: #888888; }
    QPushButton#themeBtn,
    QPushButton#settingsBtn,
    QPushButton#backBtn           { background: transparent; color: #1e1e1e;
                                    border: 1px solid #cccccc; border-radius: 6px;
                                    padding: 4px 10px; font-size: 13px; }
    QPushButton#themeBtn:hover,
    QPushButton#settingsBtn:hover,
    QPushButton#backBtn:hover     { background: #e8e8e8; }
    QRadioButton                  { background: transparent; color: #1e1e1e; font-size: 13px; }
    QRadioButton::indicator       { width: 14px; height: 14px; }
    QScrollBar:vertical           { background: #f5f5f5; width: 8px; margin: 0; }
    QScrollBar::handle:vertical   { background: #c0c0c0; border-radius: 4px; min-height: 20px; }
    QScrollBar::add-line:vertical,
    QScrollBar::sub-line:vertical { height: 0; }
)";

// ── MainWindow ────────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setupUi();
    setupWorker();
}

MainWindow::~MainWindow()
{
    m_thread->quit();
    m_thread->wait();
}

void MainWindow::setupUi()
{
    setWindowTitle("YouTube Summarizer");
    setMinimumSize(820, 640);

    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout* root = new QVBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_stack = new QStackedWidget();
    m_stack->addWidget(buildChatPage());
    m_stack->addWidget(buildSettingsPage());
    root->addWidget(m_stack);

    connect(m_sendButton,  &QPushButton::clicked,    this, &MainWindow::onSend);
    connect(m_input,       &QLineEdit::returnPressed, this, &MainWindow::onSend);
    connect(m_themeButton,         &QPushButton::clicked, this, &MainWindow::onToggleTheme);
    connect(m_themeButtonSettings, &QPushButton::clicked, this, &MainWindow::onToggleTheme);
    connect(m_settingsBtn, &QPushButton::clicked,    this, &MainWindow::onOpenSettings);
    connect(m_backBtn,     &QPushButton::clicked,    this, &MainWindow::onCloseSettings);

    applyTheme();
}

QWidget* MainWindow::buildChatPage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Header
    QWidget* header = new QWidget();
    header->setObjectName("header");
    header->setFixedHeight(52);
    QHBoxLayout* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(16, 0, 16, 0);

    QLabel* title = new QLabel("YouTube Summarizer");
    title->setObjectName("title");

    m_settingsBtn = new QPushButton("Settings");
    m_settingsBtn->setObjectName("settingsBtn");
    m_settingsBtn->setCursor(Qt::PointingHandCursor);
    m_settingsBtn->setFixedWidth(80);

    m_themeButton = new QPushButton("☀  Light");
    m_themeButton->setObjectName("themeBtn");
    m_themeButton->setCursor(Qt::PointingHandCursor);
    m_themeButton->setFixedWidth(90);

    headerLayout->addWidget(title);
    headerLayout->addStretch();
    headerLayout->addWidget(m_themeButton);
    headerLayout->addWidget(m_settingsBtn);
    layout->addWidget(header);

    // Output
    m_output = new QTextEdit();
    m_output->setObjectName("output");
    m_output->setReadOnly(true);
    layout->addWidget(m_output, 1);

    // Action bar
    QWidget* actionBar = new QWidget();
    actionBar->setObjectName("actionBar");
    actionBar->setFixedHeight(44);
    QHBoxLayout* actionLayout = new QHBoxLayout(actionBar);
    actionLayout->setContentsMargins(16, 0, 16, 0);
    actionLayout->setSpacing(16);

    QLabel* actionLabel = new QLabel("Action:");
    actionLabel->setObjectName("actionLabel");

    auto* rbSummarize = new QRadioButton("Summarize");
    auto* rbAnki      = new QRadioButton("Anki Cards");
    auto* rbBoth      = new QRadioButton("Both");
    rbSummarize->setChecked(true);

    m_actionGroup = new QButtonGroup(this);
    m_actionGroup->addButton(rbSummarize, 0);
    m_actionGroup->addButton(rbAnki,      1);
    m_actionGroup->addButton(rbBoth,      2);

    actionLayout->addWidget(actionLabel);
    actionLayout->addWidget(rbSummarize);
    actionLayout->addWidget(rbAnki);
    actionLayout->addWidget(rbBoth);
    actionLayout->addStretch();
    layout->addWidget(actionBar);

    // Input bar
    QWidget* inputBar = new QWidget();
    inputBar->setObjectName("inputBar");
    inputBar->setFixedHeight(60);
    QHBoxLayout* inputLayout = new QHBoxLayout(inputBar);
    inputLayout->setContentsMargins(12, 10, 12, 10);
    inputLayout->setSpacing(8);

    m_input = new QLineEdit();
    m_input->setObjectName("input");
    m_input->setPlaceholderText("Paste a YouTube URL or chat...");

    m_sendButton = new QPushButton("Send");
    m_sendButton->setObjectName("sendBtn");
    m_sendButton->setFixedWidth(80);
    m_sendButton->setCursor(Qt::PointingHandCursor);

    inputLayout->addWidget(m_input);
    inputLayout->addWidget(m_sendButton);
    layout->addWidget(inputBar);

    return page;
}

QWidget* MainWindow::buildSettingsPage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Header
    QWidget* header = new QWidget();
    header->setObjectName("header");
    header->setFixedHeight(52);
    QHBoxLayout* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(16, 0, 16, 0);
    headerLayout->setSpacing(12);

    m_backBtn = new QPushButton("← Back");
    m_backBtn->setObjectName("backBtn");
    m_backBtn->setCursor(Qt::PointingHandCursor);
    m_backBtn->setFixedWidth(80);

    QLabel* title = new QLabel("Settings");
    title->setObjectName("title");

    m_themeButtonSettings = new QPushButton("☀  Light");
    m_themeButtonSettings->setObjectName("themeBtn");
    m_themeButtonSettings->setCursor(Qt::PointingHandCursor);
    m_themeButtonSettings->setFixedWidth(90);

    headerLayout->addWidget(m_backBtn);
    headerLayout->addWidget(title);
    headerLayout->addStretch();
    headerLayout->addWidget(m_themeButtonSettings);
    layout->addWidget(header);

    // Empty content area
    QWidget* content = new QWidget();
    content->setObjectName("settingsContent");
    layout->addWidget(content, 1);

    return page;
}

void MainWindow::setupWorker()
{
    m_thread = new QThread(this);
    m_worker = new Worker();
    m_worker->moveToThread(m_thread);

    connect(m_thread, &QThread::finished,    m_worker, &QObject::deleteLater);
    connect(m_worker, &Worker::output,       this,     &MainWindow::onWorkerOutput);
    connect(m_worker, &Worker::done,         this,     &MainWindow::onWorkerDone);
    connect(this, &MainWindow::requestUrl,   m_worker, &Worker::processUrl);
    connect(this, &MainWindow::requestChat,  m_worker, &Worker::processChat);

    m_thread->start();
}

// ── Theme ─────────────────────────────────────────────────────────────────────

void MainWindow::applyTheme()
{
    qApp->setStyleSheet(m_darkMode ? DARK_QSS : LIGHT_QSS);
    const QString label = m_darkMode ? "☀  Light" : "☾  Dark";
    m_themeButton->setText(label);
    m_themeButtonSettings->setText(label);
}

void MainWindow::onToggleTheme()
{
    m_darkMode = !m_darkMode;
    applyTheme();
}

// ── Navigation ────────────────────────────────────────────────────────────────

void MainWindow::onOpenSettings()
{
    m_stack->setCurrentIndex(1);
}

void MainWindow::onCloseSettings()
{
    m_stack->setCurrentIndex(0);
}

// ── Output helpers ────────────────────────────────────────────────────────────

QString MainWindow::buildMessageHtml(const QString& label, const QString& labelColor,
                                     const QString& text) const
{
    QString escaped = text.toHtmlEscaped().replace("\n", "<br>");
    QString divider = m_darkMode ? "#3e3e42" : "#e0e0e0";

    return QString(
        "<div style='margin:6px 0 2px;'>"
        "<b style='color:%1;'>%2</b>"
        "</div>"
        "<div style='margin:0 0 6px 0;'>%3</div>"
        "<hr style='border:none;border-top:1px solid %4;margin:8px 0;'/>"
    ).arg(labelColor, label, escaped, divider);
}

void MainWindow::appendHtml(const QString& html)
{
    m_output->moveCursor(QTextCursor::End);
    m_output->insertHtml(html);
    m_output->verticalScrollBar()->setValue(m_output->verticalScrollBar()->maximum());
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void MainWindow::onSend()
{
    QString input = m_input->text().trimmed();
    if (input.isEmpty()) return;

    m_input->clear();
    setInputEnabled(false);

    appendHtml(buildMessageHtml("You", "#5694f1", input));

    if (URLHandler::isYouTubeUrl(input.toStdString()))
        emit requestUrl(input, m_actionGroup->checkedId());
    else
        emit requestChat(input);
}

void MainWindow::onWorkerOutput(const QString& text, const QString& type)
{
    if (type == "status")
    {
        appendHtml(QString("<p style='color:#888888;font-style:italic;margin:3px 0;'>%1</p>")
                   .arg(text.toHtmlEscaped()));
    }
    else if (type == "assistant")
    {
        appendHtml(buildMessageHtml("Assistant", "#56c7a1", text));
    }
    else if (type == "summary")
    {
        appendHtml(buildMessageHtml("Summary", "#56c7a1", text));
    }
    else if (type == "anki")
    {
        appendHtml(buildMessageHtml("Anki Flashcards", "#dcdcaa", text));
    }
    else if (type == "error")
    {
        appendHtml(QString("<p style='color:#e57373;margin:3px 0;'><b>Error:</b> %1</p>")
                   .arg(text.toHtmlEscaped()));
    }
}

void MainWindow::onWorkerDone()
{
    setInputEnabled(true);
    m_input->setFocus();
}

void MainWindow::setInputEnabled(bool enabled)
{
    m_input->setEnabled(enabled);
    m_sendButton->setEnabled(enabled);
}
