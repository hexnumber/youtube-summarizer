#pragma once
#include <QMainWindow>
#include <QButtonGroup>
#include <QThread>

class QTextEdit;
class QLineEdit;
class QPushButton;
class QComboBox;
class QStackedWidget;
class Worker;
class QClipboard;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

signals:
    void requestUrl(const QString& url, int action);
    void requestChat(const QString& message);
    void requestFetchModels();
    void requestSetModel(const QString& model);
    void requestSetSubtitleLang(const QString& lang);

private slots:
    void onSend();
    void onToggleTheme();
    void onOpenSettings();
    void onCloseSettings();
    void onWorkerOutput(const QString& text, const QString& type);
    void onWorkerDone();
    void onModelsReady(const QStringList& models, const QString& current);
    void onCopyOutput();

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

    QStackedWidget* m_stack              = nullptr;
    QTextEdit*      m_output             = nullptr;
    QString         m_lastOutput         = nullptr;
    QLineEdit*      m_input              = nullptr;
    QPushButton*    m_sendButton         = nullptr;
    QPushButton*    m_settingsBtn        = nullptr;
    QPushButton*    m_themeButton        = nullptr;
    QPushButton*    m_copyBtn            = nullptr;
    QPushButton*    m_themeButtonSettings= nullptr;
    QPushButton*    m_backBtn            = nullptr;
    QPushButton*    m_refreshBtn         = nullptr;
    QComboBox*      m_modelCombo         = nullptr;
    QComboBox*      m_langCombo          = nullptr;
    QButtonGroup*   m_actionGroup        = nullptr;

    QThread* m_thread = nullptr;
    Worker*  m_worker = nullptr;

    bool m_darkMode = true;
};
