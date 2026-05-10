#pragma once
#include <QMainWindow>
#include <QButtonGroup>
#include <QThread>

class QTextEdit;
class QLineEdit;
class QPushButton;
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
    void onWorkerOutput(const QString& text, const QString& type);
    void onWorkerDone();

private:
    void setupUi();
    void setupWorker();
    void applyTheme();
    void appendHtml(const QString& html);
    void setInputEnabled(bool enabled);
    QString buildMessageHtml(const QString& label, const QString& labelColor,
                             const QString& text) const;

    QTextEdit*    m_output;
    QLineEdit*    m_input;
    QPushButton*  m_sendButton;
    QPushButton*  m_themeButton;
    QButtonGroup* m_actionGroup;

    QThread* m_thread;
    Worker*  m_worker;

    bool m_darkMode = true;
};
