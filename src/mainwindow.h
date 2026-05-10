#pragma once
#include <QMainWindow>
#include <QButtonGroup>
#include <QThread>

class QTextEdit;
class QLineEdit;
class QPushButton;
class QStackedWidget;
class Worker;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

signals:
    void requestUrl(const QString& url, int action);
    void requestChat(const QString& message);

private slots:
    void onSend();
    void onToggleTheme();
    void onOpenSettings();
    void onCloseSettings();
    void onWorkerOutput(const QString& text, const QString& type);
    void onWorkerDone();

private:
    void     setupUi();
    void     setupWorker();
    void     applyTheme();
    void     appendHtml(const QString& html);
    void     setInputEnabled(bool enabled);
    QWidget* buildChatPage();
    QWidget* buildSettingsPage();
    QString  buildMessageHtml(const QString& label, const QString& labelColor,
                               const QString& text) const;

    QStackedWidget* m_stack       = nullptr;
    QTextEdit*      m_output      = nullptr;
    QLineEdit*      m_input       = nullptr;
    QPushButton*    m_sendButton  = nullptr;
    QPushButton*    m_settingsBtn        = nullptr;
    QPushButton*    m_themeButton        = nullptr;
    QPushButton*    m_themeButtonSettings= nullptr;
    QPushButton*    m_backBtn            = nullptr;
    QButtonGroup*   m_actionGroup = nullptr;

    QThread* m_thread = nullptr;
    Worker*  m_worker = nullptr;

    bool m_darkMode = true;
};
