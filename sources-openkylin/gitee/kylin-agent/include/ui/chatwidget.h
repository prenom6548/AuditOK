#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QJsonObject>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QPointer>
#include "models/session.h"

class QVBoxLayout;
class QHBoxLayout;
class QScrollArea;
class QLabel;
class QTextEdit;
class QPushButton;
class QComboBox;
class QSplitter;
class QTimer;
class QDialog;
class QTreeWidget;
class QFileSystemWatcher;
class ApiService;

struct ChatMessage {
    QString id;
    QString role;
    QString content;
    QString detailText;
    QString model;
    long long timestamp;
};

// Per-session chat state that persists even when switching away
struct SessionChatState {
    QVector<ChatMessage> messages;
    QString pendingUserMessage;
    bool isStreaming = false;
    QString accumulatedAssistantContent; // for delta streaming accumulation
    bool hasUnsavedAssistantContent = false;
    bool needsNewAssistantBubble = false;
    int firstUnsavedAssistantIndex = -1;
    QStringList liveDetailLines;
    QMap<QString, int> liveDetailLineByCallId;
    QStringList liveDetailFullLines;
    QMap<QString, int> liveDetailFullLineByCallId;
    QStringList liveSkillNames;
    int liveSkillLineIndex = -1;
    int liveSkillFullLineIndex = -1;
    bool liveDetailsVisible = false;
    int priorityLevel = 0;
};

struct SessionUsageStats {
    long long inputTokens = 0;
    long long outputTokens = 0;
    long long cacheReadTokens = 0;
    long long cacheWriteTokens = 0;
    long long reasoningTokens = 0;
    long long totalTokens = 0;
    int apiCalls = 0;
    long long contextTokens = 0;
    long long contextLimitTokens = 0;
    qint64 turnStartedAtMs = 0;
    qint64 lastTurnElapsedMs = 0;
    bool hasUsage = false;
    bool hasContext = false;
    bool hasTurnElapsed = false;
};

class ChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWidget(QWidget *parent = nullptr);
    ~ChatWidget();

    void loadSession(const QString &sessionId);
    void clearChat();
    void refreshModels();
    void refreshSettings();
    void reloadForDataSourceChanged(const QString &sessionId);
    QString currentSessionId() const { return m_currentSessionId; }
    void showStatus(const QString &text);
    void showNewChatButton();
    void hideNewChatButton();
    QJsonObject handleControlCommand(const QJsonObject &request);
    QJsonObject controlStatus() const;

signals:
    void messageSent(const QString &sessionId);
    void sessionChanged(const QString &sessionId);
    void sessionTitleUpdated(const QString &sessionId, const QString &title);
    void sessionExportRequested(const QString &sessionId);
    void sessionActivated(const QString &sessionId);
    void sessionStreamingChanged(const QString &sessionId, bool streaming);
    void sessionMarkedUnread(const QString &sessionId);

private slots:
    void sendMessage();
    void cancelGeneration();
    void onChatCompletionDelta(const QString &sessionId, const QString &delta);
    void onChatCompletionFinished(const QString &sessionId, const QString &content, bool ok);

private:
    void setupUi();
    void setupConnections();
    void addMessage(const QString &role, const QString &content);
    void appendToLastMessage(const QString &content);
    void clearMessagesLayout();
    void rebuildMessagesLayout();
    void insertMessageToLayout(const ChatMessage &msg);
    void scrollToBottom();
    void clearMessages();
    void refreshMessages();
    QLabel *addMessageBubble(const QString &role, const QString &content);
    QString selectedModelName() const;
    QString localAssistantReply(const QString &userText) const;
    void updateRequestButtons();
    void scheduleScrollToBottom(bool force = false);
    bool shouldAutoScrollToBottom() const;

    void loadMessagesForSession(const QString &sessionId);
    void loadModels();
    void positionExportButton();
    void syncSelectedModelToConfig();
    void updateDetailsButtons();
    void updateAccelerationButton();
    void updateUsageLabel();
    void refreshUsageStatsForSession(const QString &sessionId);
    void applyUsageDetailEvent(const QString &sessionId, const QJsonObject &event);
    void applyUsageTextLines(const QString &sessionId, const QString &details);
    bool refreshUsageStatsFromUsageLog(const QString &sessionId);
    void watchUsageLogForSession(const QString &sessionId);
    void scheduleUsageLogRefresh(const QString &sessionId);
    bool handleAttachmentDropEvent(QObject *obj, QEvent *event);
    void addPendingAttachmentPath(const QString &filePath);
    void removePendingAttachmentPath(const QString &filePath);
    void clearPendingAttachments();
    void updateAttachmentChips();
    void exportSessionDetails(const QString &sessionId);
    void exportDetailsPreviewRows(const QString &sessionId, QTreeWidget *tree, QComboBox *modeSelector);
    void previewSessionDetails(const QString &sessionId);
    QString buildMessageDetailsText(const ChatMessage &message);
    void showDetailsPreview(const QString &title, const QString &sessionId);
    void addMessageActionButtons(QVBoxLayout *msgLayout, const ChatMessage &message, QLabel *contentLabel);
    void ensureLiveDetailBubble(const QString &sessionId);
    void updateLiveDetailBubble(const QString &sessionId);
    void finalizeLiveDetailsForSession(const QString &sessionId);
    void attachLiveDetailsToMessage(const QString &sessionId, int messageIndex);
    void persistUnsavedAssistantMessages(const QString &sessionId);
    void resetLiveDetailsForSession(const QString &sessionId);
    void collapseLiveDetailBubble(const QString &sessionId);

    // Per-session state management
    void saveCurrentSessionState();
    void loadSessionState(const QString &sessionId);
    void updateSendButtonForSession(const QString &sessionId);
    void updateSendButtonState();

    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_headerLayout;
    QLabel *m_titleLabel;
    QLabel *m_sessionTitleLabel;
    QComboBox *m_modelCombo;
    QComboBox *m_modelSelector;
    QPushButton *m_cancelBtn;
    QComboBox *m_prioritySelector;
    QLabel *m_usageLabel;
    QPushButton *m_previewBtn;
    QPushButton *m_exportBtn;
    QScrollArea *m_messagesArea;
    QWidget *m_messagesContainer;
    QVBoxLayout *m_messagesLayout;
    QHBoxLayout *m_inputLayout;
    QTextEdit *m_inputEdit;
    QWidget *m_attachmentContainer;
    QHBoxLayout *m_attachmentLayout;
    QPushButton *m_sendBtn;
    QPushButton *m_sendButton;
    QPushButton *m_stopButton;
    QWidget *m_toolbarWidget;
    QPushButton *m_newChatBtn;
    QWidget *m_liveDetailWrapper;
    QLabel *m_liveDetailLabel;
    QTimer *m_liveDetailTimer;
    QTimer *m_usageTimer;
    QFileSystemWatcher *m_usageLogWatcher;
    int m_liveDetailDotCount;
    QPointer<QDialog> m_detailsPreviewDialog;
    bool m_userScrolledAwayFromBottom;

    QString m_currentSessionId;
    QString m_watchedUsageSessionId;
    QString m_currentModel;
    QStringList m_pendingAttachmentPaths;
    QMap<QString, QString> m_pendingFallbackReplies;
    QMap<QString, QString> m_streamedReplies;
    QMap<QString, QLabel *> m_streamBubbles;
    bool m_isStreaming;
    bool m_needsNewAssistantBubble;
    ApiService *m_apiService;
    QVector<ChatMessage> m_messages;
    QString m_pendingUserMessage;

    // Map of session ID -> per-session state
    QMap<QString, SessionChatState> m_sessionStates;
    QMap<QString, SessionUsageStats> m_sessionUsageStats;
};

#endif // CHATWIDGET_H
