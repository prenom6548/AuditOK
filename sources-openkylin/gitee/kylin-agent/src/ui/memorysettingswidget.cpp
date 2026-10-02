#include "ui/memorysettingswidget.h"

#include "services/apiservice.h"

#include <QFrame>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QPainter>
#include <QPaintEvent>
#include <QTimer>
#include <kmessagebox.h>

namespace {

class PlaceholderPlainTextEdit : public QPlainTextEdit
{
public:
    explicit PlaceholderPlainTextEdit(QWidget *parent = nullptr)
        : QPlainTextEdit(parent)
    {
        QPlainTextEdit::setPlaceholderText(QString());
    }

    void setGrayPlaceholderText(const QString &text)
    {
        m_placeholderText = text;
        viewport()->update();
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QPlainTextEdit::paintEvent(event);
        if (!document()->isEmpty() || m_placeholderText.isEmpty()) {
            return;
        }
        QPainter painter(viewport());
        painter.setPen(QColor(QStringLiteral("#8a8a8a")));
        painter.drawText(viewport()->rect().adjusted(14, 12, -14, -12),
                         Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                         m_placeholderText);
    }

private:
    QString m_placeholderText;
};

QLabel *makeStatLabel(const QString &value, const QString &label, QWidget *parent)
{
    QLabel *widget = new QLabel(
        QStringLiteral("<b>%1</b> <span style='color:#8a8a8a;'>%2</span>")
            .arg(value.toHtmlEscaped(), label.toHtmlEscaped()),
        parent);
    widget->setTextFormat(Qt::RichText);
    widget->setObjectName(QStringLiteral("statLabel"));
    return widget;
}

} // namespace

MemorySettingsWidget::MemorySettingsWidget(QWidget *parent)
    : QWidget(parent),
      m_mainLayout(new QVBoxLayout(this)),
      m_sessionsValue(nullptr),
      m_messagesValue(nullptr),
      m_profileEntriesValue(nullptr),
      m_editor(nullptr),
      m_hintLabel(nullptr),
      m_charLabel(nullptr),
      m_refreshButton(nullptr),
      m_saveButton(nullptr)
{
    createMemoryGroup();
    refreshMemory();
}

void MemorySettingsWidget::createMemoryGroup()
{
    setObjectName(QStringLiteral("memorySettingsWidget"));
    setMinimumWidth(520);
    m_mainLayout->setContentsMargins(28, 24, 28, 28);
    m_mainLayout->setSpacing(16);

    setStyleSheet(R"(
        #memorySettingsWidget { background: palette(window); color: palette(window-text); }
        QLabel#pageTitle { font-size: 22px; font-weight: 700; color: palette(window-text); }
        QLabel#pageSubtitle { font-size: 14px; color: #8a8a8a; }
        QPushButton#refreshButton {
            background: palette(button); border: 1px solid palette(mid);
            border-radius: 10px; padding: 8px 14px; font-weight: 700;
        }
        QPushButton#refreshButton:disabled { background: palette(midlight); color: #8a8a8a; }
        QFrame#statsCard { background: palette(base); border: 1px solid palette(mid); border-radius: 8px; }
        QLabel#statLabel { font-size: 14px; }
        QPlainTextEdit#memoryEditor {
            background: palette(base); border: 1px solid palette(mid); border-radius: 8px;
            padding: 14px; font-size: 14px; color: palette(text);
        }
        QPlainTextEdit#memoryEditor:focus { border-color: #8aa9d6; }
        QLabel#hintLabel, QLabel#charLabel { color: #8a8a8a; font-size: 12px; }
        QPushButton#saveButton {
            background: #4a90d9; color: #ffffff; border: none;
            border-radius: 8px; padding: 7px 14px; font-weight: 700;
        }
        QPushButton#saveButton:hover { background: #3d7fc4; }
        QPushButton#saveButton:pressed { background: #326cab; }
        QPushButton#saveButton:disabled {
            background: palette(midlight); color: #8a8a8a;
            border: 1px solid palette(mid);
        }
    )");

    QHBoxLayout *header = new QHBoxLayout();
    QVBoxLayout *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(8);
    QLabel *title = new QLabel(tr("记忆"), this);
    title->setObjectName(QStringLiteral("pageTitle"));
    titleLayout->addWidget(title);
    QLabel *subtitle = new QLabel(tr("查看和编辑智能体用于个性化回答的用户画像。"), this);
    subtitle->setObjectName(QStringLiteral("pageSubtitle"));
    titleLayout->addWidget(subtitle);
    header->addLayout(titleLayout, 1);
    m_refreshButton = new QPushButton(tr("↻ 刷新"), this);
    m_refreshButton->setObjectName(QStringLiteral("refreshButton"));
    m_refreshButton->setCursor(Qt::PointingHandCursor);
    connect(m_refreshButton, &QPushButton::clicked,
            this, &MemorySettingsWidget::refreshMemoryWithFeedback);
    header->addWidget(m_refreshButton, 0, Qt::AlignTop);
    m_mainLayout->addLayout(header);

    QFrame *statsCard = new QFrame(this);
    statsCard->setObjectName(QStringLiteral("statsCard"));
    QHBoxLayout *stats = new QHBoxLayout(statsCard);
    stats->setContentsMargins(20, 16, 20, 16);
    stats->setSpacing(28);
    m_sessionsValue = makeStatLabel(QStringLiteral("0"), tr("会话"), statsCard);
    m_messagesValue = makeStatLabel(QStringLiteral("0"), tr("消息"), statsCard);
    m_profileEntriesValue = makeStatLabel(QStringLiteral("0"), tr("画像字段"), statsCard);
    stats->addWidget(m_sessionsValue);
    stats->addWidget(m_messagesValue);
    stats->addWidget(m_profileEntriesValue);
    stats->addStretch(1);
    m_mainLayout->addWidget(statsCard);

    QHBoxLayout *editorHeader = new QHBoxLayout();
    QLabel *editorTitle = new QLabel(tr("用户画像"), this);
    editorTitle->setStyleSheet(QStringLiteral("font-weight: 700;"));
    editorHeader->addWidget(editorTitle);
    editorHeader->addStretch(1);
    m_saveButton = new QPushButton(tr("保存"), this);
    m_saveButton->setObjectName(QStringLiteral("saveButton"));
    m_saveButton->setEnabled(false);
    connect(m_saveButton, &QPushButton::clicked, this, &MemorySettingsWidget::saveCurrentEditor);
    editorHeader->addWidget(m_saveButton);
    m_mainLayout->addLayout(editorHeader);

    m_hintLabel = new QLabel(
        tr("画像由运行时自动提取并加密保存。这里显示可编辑的 JSON 字段；键为画像路径，值可以是文本、数字、布尔值或数组。"),
        this);
    m_hintLabel->setObjectName(QStringLiteral("hintLabel"));
    m_hintLabel->setWordWrap(true);
    m_mainLayout->addWidget(m_hintLabel);

    m_editor = new PlaceholderPlainTextEdit(this);
    m_editor->setObjectName(QStringLiteral("memoryEditor"));
    m_editor->setMinimumHeight(320);
    static_cast<PlaceholderPlainTextEdit *>(m_editor)->setGrayPlaceholderText(
        tr("暂无用户画像。开始对话后，运行时会自动提取画像；也可以在此填写 JSON 对象。"));
    connect(m_editor, &QPlainTextEdit::textChanged, this, [this]() {
        if (m_loading) {
            return;
        }
        m_charLabel->setText(tr("%1 字符").arg(m_editor->toPlainText().length()));
        m_saveButton->setEnabled(m_profileAvailable && m_editor->toPlainText() != m_savedContent);
    });
    m_mainLayout->addWidget(m_editor, 1);

    QHBoxLayout *footer = new QHBoxLayout();
    m_charLabel = new QLabel(this);
    m_charLabel->setObjectName(QStringLiteral("charLabel"));
    footer->addWidget(m_charLabel);
    footer->addStretch(1);
    m_mainLayout->addLayout(footer);
}

void MemorySettingsWidget::refreshMemory()
{
    const QJsonObject profile = ApiService::instance().getUserProfileSync();
    m_profileAvailable = profile.contains(QStringLiteral("entries")) &&
                         profile.value(QStringLiteral("entries")).isObject();
    m_profileEntries = m_profileAvailable
        ? profile.value(QStringLiteral("entries")).toObject()
        : QJsonObject();
    m_profileUpdatedAt = profile.value(QStringLiteral("updated_at")).toString();
    updateStats();
    updateEditor();
}

void MemorySettingsWidget::refreshMemoryWithFeedback()
{
    if (!m_refreshButton || !m_refreshButton->isEnabled()) {
        return;
    }
    m_refreshButton->setEnabled(false);
    m_refreshButton->setText(tr("↻ 刷新中…"));
    QTimer::singleShot(0, this, [this]() {
        refreshMemory();
        m_refreshButton->setText(m_profileAvailable ? tr("✓ 已刷新") : tr("刷新失败"));
        QTimer::singleShot(1000, this, [this]() {
            if (m_refreshButton) {
                m_refreshButton->setText(tr("↻ 刷新"));
                m_refreshButton->setEnabled(true);
            }
        });
    });
}

void MemorySettingsWidget::updateEditor()
{
    m_loading = true;
    const QString content = m_profileAvailable
        ? QString::fromUtf8(QJsonDocument(m_profileEntries).toJson(QJsonDocument::Indented)).trimmed()
        : QString();
    m_editor->setPlainText(content);
    m_editor->setReadOnly(!m_profileAvailable);
    m_savedContent = content;
    m_charLabel->setText(tr("%1 字符").arg(content.length()));
    m_saveButton->setEnabled(false);
    if (!m_profileAvailable) {
        m_hintLabel->setText(tr("无法连接用户画像接口。请确认 agent-runtime 已更新并正在运行。"));
    } else {
        m_hintLabel->setText(
            tr("画像由运行时自动提取并加密保存。这里显示可编辑的 JSON 字段；删除字段会从画像中移除它。"));
    }
    m_loading = false;
}

void MemorySettingsWidget::updateStats()
{
    const QMap<QString, QVariant> usage = ApiService::instance().getUsageSync();
    m_sessionsValue->setText(
        QStringLiteral("<b>%1</b> <span style='color:#8a8a8a;'>%2</span>")
            .arg(usage.value(QStringLiteral("sessions")).toInt()).arg(tr("会话")));
    m_messagesValue->setText(
        QStringLiteral("<b>%1</b> <span style='color:#8a8a8a;'>%2</span>")
            .arg(usage.value(QStringLiteral("messages")).toInt()).arg(tr("消息")));
    m_profileEntriesValue->setText(
        QStringLiteral("<b>%1</b> <span style='color:#8a8a8a;'>%2</span>")
            .arg(m_profileEntries.size()).arg(tr("画像字段")));
}

void MemorySettingsWidget::saveCurrentEditor()
{
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(
        m_editor->toPlainText().toUtf8(), &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        kdk::KMessageBox::warning(
            this, tr("JSON 格式错误"),
            tr("用户画像必须是 JSON 对象。%1（位置 %2）")
                .arg(error.errorString()).arg(error.offset));
        return;
    }

    if (!ApiService::instance().putUserProfileSync(document.object(), m_profileUpdatedAt)) {
        kdk::KMessageBox::warning(
            this, tr("保存失败"),
            tr("无法保存用户画像。画像可能已被对话更新，请刷新后重试。"));
        return;
    }
    refreshMemory();

    kdk::KMessageBox savedMessage(window());
    savedMessage.setIcon(kdk::KMessageBox::Information);
    savedMessage.setWindowTitle(tr("已保存"));
    savedMessage.setText(tr("用户画像已保存。"));
    QPushButton *savedCloseButton = savedMessage.addButton(kdk::KMessageBox::Close);
    savedCloseButton->setText(tr("关闭"));
    QTimer::singleShot(1500, &savedMessage, &kdk::KMessageBox::accept);
    savedMessage.exec();
}

void MemorySettingsWidget::loadSettings()
{
    refreshMemory();
}

void MemorySettingsWidget::saveSettings()
{
    if (m_saveButton->isEnabled()) {
        saveCurrentEditor();
    }
}
