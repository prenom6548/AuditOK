#include "ui/chatwidget.h"

#include "services/apiservice.h"
#include "services/configservice.h"
#include "services/modelservice.h"
#include "services/sessionservice.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QTextEdit>
#include <QToolTip>
#include <QVBoxLayout>

ChatWidget::ChatWidget(QWidget *parent)
    : QWidget(parent)
    , m_titleLabel(nullptr)
    , m_modelCombo(nullptr)
    , m_messagesArea(nullptr)
    , m_messagesContainer(nullptr)
    , m_messagesLayout(nullptr)
    , m_inputEdit(nullptr)
    , m_sendButton(nullptr)
    , m_stopButton(nullptr)
{
    setupUi();
    connect(&ApiService::instance(), &ApiService::chatCompletionFinished,
            this, &ChatWidget::onChatCompletionFinished);
    connect(&ApiService::instance(), &ApiService::chatCompletionDelta,
            this, &ChatWidget::onChatCompletionDelta);
}

ChatWidget::~ChatWidget() = default;

void ChatWidget::setupUi()
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    QWidget *header = new QWidget(this);
    header->setFixedHeight(48);
    header->setStyleSheet(QStringLiteral("border-bottom: 1px solid #dfe3ea; background: #ffffff;"));
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(18, 0, 18, 0);
    m_titleLabel = new QLabel(QStringLiteral("No session selected"), header);
    m_titleLabel->setStyleSheet(QStringLiteral("font-weight: 600; color: #20242a;"));
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addStretch();

    m_modelCombo = new QComboBox(header);
    m_modelCombo->setMinimumWidth(220);
    m_modelCombo->setToolTip(QStringLiteral("Model"));
    connect(m_modelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (index >= 0) {
            ConfigService::instance().setSelectedModelId(m_modelCombo->itemData(index).toString());
        }
    });
    headerLayout->addWidget(m_modelCombo);
    root->addWidget(header);

    m_messagesArea = new QScrollArea(this);
    m_messagesArea->setWidgetResizable(true);
    m_messagesArea->setFrameShape(QFrame::NoFrame);
    m_messagesContainer = new QWidget(m_messagesArea);
    m_messagesLayout = new QVBoxLayout(m_messagesContainer);
    m_messagesLayout->setContentsMargins(24, 24, 24, 24);
    m_messagesLayout->setSpacing(12);
    m_messagesLayout->addStretch();
    m_messagesArea->setWidget(m_messagesContainer);
    root->addWidget(m_messagesArea, 1);

    QWidget *inputBar = new QWidget(this);
    inputBar->setStyleSheet(QStringLiteral("border-top: 1px solid #dfe3ea; background: #ffffff;"));
    QHBoxLayout *inputLayout = new QHBoxLayout(inputBar);
    inputLayout->setContentsMargins(16, 12, 16, 12);
    inputLayout->setSpacing(10);

    m_inputEdit = new QTextEdit(inputBar);
    m_inputEdit->setFixedHeight(72);
    m_inputEdit->setPlaceholderText(QStringLiteral("Type a message..."));
    inputLayout->addWidget(m_inputEdit, 1);

    m_sendButton = new QPushButton(QStringLiteral("Send"), inputBar);
    m_sendButton->setFixedSize(88, 40);
    connect(m_sendButton, &QPushButton::clicked, this, &ChatWidget::sendMessage);
    inputLayout->addWidget(m_sendButton);

    m_stopButton = new QPushButton(QStringLiteral("Stop"), inputBar);
    m_stopButton->setFixedSize(88, 40);
    m_stopButton->setEnabled(false);
    connect(m_stopButton, &QPushButton::clicked, this, &ChatWidget::cancelGeneration);
    inputLayout->addWidget(m_stopButton);
    root->addWidget(inputBar);

    loadModels();
    connect(&ModelService::instance(), &ModelService::modelsChanged, this, &ChatWidget::loadModels);
}

void ChatWidget::loadModels()
{
    if (!m_modelCombo) {
        return;
    }

    const QString selectedId = ConfigService::instance().selectedModelId();
    m_modelCombo->blockSignals(true);
    m_modelCombo->clear();

    int selectedIndex = -1;
    const QVector<SavedModel> models = ModelService::instance().listModels();
    for (const SavedModel &model : models) {
        const QString label = model.name().isEmpty() ? model.model() : model.name();
        m_modelCombo->addItem(label, model.id());
        m_modelCombo->setItemData(m_modelCombo->count() - 1, model.model(), Qt::UserRole + 1);
        if (!selectedId.isEmpty() && model.id() == selectedId) {
            selectedIndex = m_modelCombo->count() - 1;
        }
    }

    if (m_modelCombo->count() == 0) {
        m_modelCombo->addItem(QStringLiteral("default"), QString());
        m_modelCombo->setItemData(0, QStringLiteral("default"), Qt::UserRole + 1);
        selectedIndex = 0;
    } else if (selectedIndex < 0) {
        selectedIndex = 0;
    }

    m_modelCombo->setCurrentIndex(selectedIndex);
    m_modelCombo->blockSignals(false);
    ConfigService::instance().setSelectedModelId(m_modelCombo->itemData(selectedIndex).toString());
}

QString ChatWidget::selectedModelName() const
{
    if (!m_modelCombo || m_modelCombo->currentIndex() < 0) {
        return QString();
    }

    return m_modelCombo->itemData(m_modelCombo->currentIndex(), Qt::UserRole + 1).toString();
}

void ChatWidget::loadSession(const QString &sessionId)
{
    m_currentSessionId = sessionId;
    m_titleLabel->setText(QStringLiteral("Session %1").arg(sessionId.left(8)));
    updateRequestButtons();
    refreshMessages();
}

void ChatWidget::clearChat()
{
    if (!m_currentSessionId.isEmpty()) {
        m_pendingFallbackReplies.remove(m_currentSessionId);
        m_streamedReplies.remove(m_currentSessionId);
        m_streamBubbles.remove(m_currentSessionId);
    }

    m_currentSessionId.clear();
    if (m_titleLabel) {
        m_titleLabel->setText(QStringLiteral("New session"));
    }
    clearMessages();
    if (m_inputEdit) {
        m_inputEdit->clear();
    }
    updateRequestButtons();
}

void ChatWidget::showStatus(const QString &text)
{
    clearMessages();
    m_titleLabel->setText(QStringLiteral("Kylin-Agent"));
    addMessageBubble(QStringLiteral("system"), text);
}

void ChatWidget::clearMessages()
{
    m_streamBubbles.clear();
    while (QLayoutItem *item = m_messagesLayout->takeAt(0)) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    m_messagesLayout->addStretch();
}

void ChatWidget::refreshMessages()
{
    clearMessages();
    const QVector<SessionMessage> messages = SessionService::instance().getSessionMessages(m_currentSessionId);
    if (messages.isEmpty()) {
        addMessageBubble(QStringLiteral("system"), QStringLiteral("This session has no messages yet."));
        return;
    }

    for (const SessionMessage &message : messages) {
        addMessageBubble(message.role(), message.content());
    }
    scrollToBottom();
}

QLabel *ChatWidget::addMessageBubble(const QString &role, const QString &content)
{
    QLabel *bubble = new QLabel(content, m_messagesContainer);
    bubble->setWordWrap(true);
    bubble->setTextInteractionFlags(Qt::TextSelectableByMouse);
    bubble->setContextMenuPolicy(Qt::CustomContextMenu);
    bubble->setContentsMargins(12, 10, 12, 10);

    const bool isUser = role == QStringLiteral("user");
    const QString background = isUser ? QStringLiteral("#dff6e8") : QStringLiteral("#eef3fb");
    bubble->setStyleSheet(QStringLiteral(
        "QLabel {"
        "  background: %1;"
        "  border: 1px solid #d7dde7;"
        "  border-radius: 6px;"
        "  color: #20242a;"
        "}"
    ).arg(background));
    connect(bubble, &QLabel::customContextMenuRequested, this, [this, bubble](const QPoint &pos) {
        QMenu menu(bubble);
        QAction *copyAction = menu.addAction(QStringLiteral("Copy"));
        copyAction->setEnabled(!bubble->text().isEmpty());
        QAction *selected = menu.exec(bubble->mapToGlobal(pos));
        if (selected == copyAction) {
            QApplication::clipboard()->setText(bubble->text());
            QToolTip::showText(bubble->mapToGlobal(pos), QStringLiteral("Copied"));
        }
    });

    QHBoxLayout *row = new QHBoxLayout;
    row->setContentsMargins(0, 0, 0, 0);
    if (isUser) {
        row->addStretch();
        row->addWidget(bubble, 0);
    } else {
        row->addWidget(bubble, 0);
        row->addStretch();
    }

    m_messagesLayout->insertLayout(qMax(0, m_messagesLayout->count() - 1), row);
    return bubble;
}

QString ChatWidget::localAssistantReply(const QString &userText) const
{
    Q_UNUSED(userText)
    return QStringLiteral("已保存到本机会话。agent-rt 接入后，这里将显示后端回复。");
}

void ChatWidget::updateRequestButtons()
{
    const bool pending = m_pendingFallbackReplies.contains(m_currentSessionId);
    m_sendButton->setEnabled(!pending);
    m_stopButton->setEnabled(pending);
}

QJsonObject ChatWidget::controlStatus() const
{
    QJsonArray runningSessions;
    if (!m_currentSessionId.isEmpty() && m_pendingFallbackReplies.contains(m_currentSessionId)) {
        runningSessions.append(m_currentSessionId);
    }

    return {
        {"session_id", m_currentSessionId},
        {"input", m_inputEdit ? m_inputEdit->toPlainText() : QString()},
        {"current_running", !m_currentSessionId.isEmpty()
            && m_pendingFallbackReplies.contains(m_currentSessionId)},
        {"running", !runningSessions.isEmpty()},
        {"running_session_ids", runningSessions},
        {"running_count", runningSessions.size()},
        {"queued_count", 0},
        {"has_pending_tasks", !runningSessions.isEmpty()},
        {"messages_loading", false},
        {"model_available", m_modelCombo && m_modelCombo->count() > 0}
    };
}

QJsonObject ChatWidget::handleControlCommand(const QJsonObject &request)
{
    const auto fail = [](const QString &code, const QString &message) {
        return QJsonObject{
            {"ok", false},
            {"error", QJsonObject{{"code", code}, {"message", message}}}
        };
    };

    const QString command = request.value(QStringLiteral("command")).toString();
    if (command == QStringLiteral("running_count")) {
        const QJsonObject status = controlStatus();
        return {{"ok", true}, {"result", QJsonObject{
            {"running_count", status.value(QStringLiteral("running_count"))},
            {"running_session_ids", status.value(QStringLiteral("running_session_ids"))}
        }}};
    }

    if (command != QStringLiteral("status") && command != QStringLiteral("new_session") &&
        command != QStringLiteral("set_input") && command != QStringLiteral("send") &&
        command != QStringLiteral("stop")) {
        return fail(QStringLiteral("unknown_command"),
                    QStringLiteral("Expected status, running_count, new_session, set_input, send or stop"));
    }

    if (request.contains(QStringLiteral("session_id")) &&
        (!request.value(QStringLiteral("session_id")).isString() ||
         request.value(QStringLiteral("session_id")).toString() != m_currentSessionId)) {
        return fail(QStringLiteral("session_mismatch"),
                    QStringLiteral("Current session differs from session_id"));
    }

    if ((command == QStringLiteral("set_input") ||
         (command == QStringLiteral("send") && request.contains(QStringLiteral("text")))) &&
        !request.value(QStringLiteral("text")).isString()) {
        return fail(QStringLiteral("invalid_argument"), QStringLiteral("text must be a string"));
    }

    if (command == QStringLiteral("new_session")) {
        clearChat();
        if (m_inputEdit) {
            m_inputEdit->clear();
        }
    } else if (command == QStringLiteral("set_input")) {
        m_inputEdit->setPlainText(request.value(QStringLiteral("text")).toString());
    } else if (command == QStringLiteral("send")) {
        if (!m_currentSessionId.isEmpty() && m_pendingFallbackReplies.contains(m_currentSessionId)) {
            return fail(QStringLiteral("task_running"),
                        QStringLiteral("Current session already has a running task"));
        }
        if (request.contains(QStringLiteral("text"))) {
            const QString text = request.value(QStringLiteral("text")).toString();
            if (text.trimmed().isEmpty()) {
                return fail(QStringLiteral("empty_input"), QStringLiteral("text must not be empty"));
            }
            m_inputEdit->setPlainText(text);
        } else if (!m_inputEdit || m_inputEdit->toPlainText().trimmed().isEmpty()) {
            return fail(QStringLiteral("empty_input"), QStringLiteral("Input is empty"));
        }
        sendMessage();
        if (m_currentSessionId.isEmpty()) {
            return fail(QStringLiteral("backend_error"), QStringLiteral("Failed to create session"));
        }
    } else if (command == QStringLiteral("stop")) {
        cancelGeneration();
    }

    QJsonObject result = controlStatus();
    if (command == QStringLiteral("send")) {
        result.insert(QStringLiteral("accepted"), true);
    }
    if (command == QStringLiteral("stop")) {
        result.insert(QStringLiteral("stop_requested"), true);
    }
    return {{"ok", true}, {"result", result}};
}

static QJsonArray buildChatMessages(const QVector<SessionMessage> &history)
{
    QJsonArray messages;
    for (const SessionMessage &message : history) {
        if (message.content().trimmed().isEmpty()) {
            continue;
        }

        QJsonObject item;
        item.insert(QStringLiteral("role"), message.role());
        item.insert(QStringLiteral("content"), message.content());
        messages.append(item);
    }
    return messages;
}

void ChatWidget::sendMessage()
{
    QString text = m_inputEdit->toPlainText().trimmed();
    if (text.isEmpty()) {
        return;
    }

    if (m_currentSessionId.isEmpty()) {
        m_currentSessionId = SessionService::instance().createSession(QStringLiteral("desktop"), QString());
        if (m_currentSessionId.isEmpty()) {
            showStatus(QStringLiteral("Failed to create a local session."));
            return;
        }
        m_titleLabel->setText(QStringLiteral("Session %1").arg(m_currentSessionId.left(8)));
    }

    if (!SessionService::instance().addMessage(m_currentSessionId, QStringLiteral("user"), text)) {
        return;
    }

    m_inputEdit->clear();
    refreshMessages();

    const QJsonArray messages = buildChatMessages(SessionService::instance().getSessionMessages(m_currentSessionId));
    m_pendingFallbackReplies.insert(m_currentSessionId, localAssistantReply(text));
    updateRequestButtons();
    ApiService::instance().sendChatCompletion(m_currentSessionId, messages, selectedModelName());
    emit messageSent(m_currentSessionId);
}

void ChatWidget::cancelGeneration()
{
    if (m_currentSessionId.isEmpty() || !m_pendingFallbackReplies.contains(m_currentSessionId)) {
        return;
    }

    m_pendingFallbackReplies.insert(m_currentSessionId, QStringLiteral("生成已停止。"));
    m_streamedReplies.remove(m_currentSessionId);
    m_streamBubbles.remove(m_currentSessionId);
    ApiService::instance().cancelChatCompletion(m_currentSessionId);
}

void ChatWidget::onChatCompletionDelta(const QString &sessionId, const QString &delta)
{
    if (!m_pendingFallbackReplies.contains(sessionId)) {
        return;
    }

    m_streamedReplies[sessionId].append(delta);
    m_pendingFallbackReplies.insert(sessionId, m_streamedReplies.value(sessionId));

    if (sessionId != m_currentSessionId) {
        return;
    }

    QLabel *bubble = m_streamBubbles.value(sessionId, nullptr);
    if (!bubble) {
        bubble = addMessageBubble(QStringLiteral("assistant"), QString());
        m_streamBubbles.insert(sessionId, bubble);
    }
    bubble->setText(m_streamedReplies.value(sessionId));
    scrollToBottom();
}

void ChatWidget::onChatCompletionFinished(const QString &sessionId, const QString &content, bool ok)
{
    if (!m_pendingFallbackReplies.contains(sessionId)) {
        return;
    }

    QString assistantReply = ok ? content.trimmed() : QString();
    if (assistantReply.isEmpty()) {
        assistantReply = m_streamedReplies.value(sessionId).trimmed();
    }
    if (assistantReply.isEmpty()) {
        assistantReply = m_pendingFallbackReplies.value(sessionId);
    }
    m_pendingFallbackReplies.remove(sessionId);
    m_streamedReplies.remove(sessionId);
    m_streamBubbles.remove(sessionId);

    SessionService::instance().addMessage(sessionId, QStringLiteral("assistant"), assistantReply);
    if (sessionId == m_currentSessionId) {
        refreshMessages();
        updateRequestButtons();
    }
    emit messageSent(sessionId);
}

void ChatWidget::scrollToBottom()
{
    if (!m_messagesArea || !m_messagesArea->verticalScrollBar()) {
        return;
    }
    m_messagesArea->verticalScrollBar()->setValue(m_messagesArea->verticalScrollBar()->maximum());
}
