#include "ui/personasettingswidget.h"
#include "services/apiservice.h"

#include <QHBoxLayout>
#include <kmessagebox.h>
#include <QTimer>

PersonaSettingsWidget::PersonaSettingsWidget(QWidget *parent)
    : QWidget(parent),
      m_mainLayout(new QVBoxLayout(this)),
      m_editor(nullptr),
      m_saveButton(nullptr),
      m_resetButton(nullptr) {
    createPersonaGroup();
    loadSettings();
}

void PersonaSettingsWidget::createPersonaGroup() {
    setObjectName(QStringLiteral("personaSettingsWidget"));
    setMinimumWidth(520);
    m_mainLayout->setContentsMargins(28, 24, 28, 28);
    m_mainLayout->setSpacing(16);

    setStyleSheet(R"(
        #personaSettingsWidget { background: palette(window); color: palette(window-text); }
        QLabel#pageTitle { font-size: 22px; font-weight: 700; color: palette(window-text); }
        QLabel#pageSubtitle { font-size: 14px; color: #8a8a8a; }
        QPushButton#saveButton {
            background: #4a90d9;
            border: none;
            border-radius: 10px;
            padding: 8px 16px;
            font-weight: 700;
            color: #ffffff;
        }
        QPushButton#saveButton:hover { background: #3d7fc4; }
        QPushButton#saveButton:pressed { background: #326cab; }
        QPushButton#saveButton:disabled {
            background: palette(midlight);
            border: 1px solid palette(mid);
            color: #8a8a8a;
        }
        QPushButton#resetButton {
            background: palette(button);
            border: 1px solid palette(mid);
            border-radius: 10px;
            padding: 8px 16px;
            font-weight: 700;
            color: palette(text);
        }
        QPushButton#resetButton:hover { background: palette(midlight); }
        QPlainTextEdit#personaEditor {
            background: palette(base);
            border: 1px solid #214a85;
            border-radius: 8px;
            padding: 16px;
            font-family: "Monospace";
            font-size: 14px;
            line-height: 170%;
            color: palette(window-text);
        }
        QLabel#hintLabel { color: #8a8a8a; font-size: 12px; }
    )");

    QHBoxLayout *header = new QHBoxLayout();
    header->setContentsMargins(0, 0, 0, 0);
    header->setSpacing(12);

    QVBoxLayout *titleLayout = new QVBoxLayout();
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(8);

    QHBoxLayout *titleRow = new QHBoxLayout();
    QLabel *title = new QLabel(tr("个性"), this);
    title->setObjectName(QStringLiteral("pageTitle"));
    titleRow->addWidget(title);
    titleRow->addStretch(1);
    titleLayout->addLayout(titleRow);

    QLabel *subtitle = new QLabel(tr("通过 SOUL.md 定义智能体的人格、语气和长期指令"), this);
    subtitle->setObjectName(QStringLiteral("pageSubtitle"));
    titleLayout->addWidget(subtitle);
    header->addLayout(titleLayout, 1);

    m_saveButton = new QPushButton(tr("保存"), this);
    m_saveButton->setObjectName(QStringLiteral("saveButton"));
    m_saveButton->setCursor(Qt::PointingHandCursor);
    m_saveButton->setEnabled(false);
    connect(m_saveButton, &QPushButton::clicked, this, [this]() { savePersona(true); });
    header->addWidget(m_saveButton, 0, Qt::AlignTop);

    m_resetButton = new QPushButton(tr("重置"), this);
    m_resetButton->setObjectName(QStringLiteral("resetButton"));
    m_resetButton->setCursor(Qt::PointingHandCursor);
    connect(m_resetButton, &QPushButton::clicked, this, &PersonaSettingsWidget::resetPersona);
    header->addWidget(m_resetButton, 0, Qt::AlignTop);
    m_mainLayout->addLayout(header);

    m_editor = new QPlainTextEdit(this);
    m_editor->setObjectName(QStringLiteral("personaEditor"));
    m_editor->setMinimumHeight(520);
    m_editor->setPlaceholderText(tr("填写智能体的人格、沟通风格和长期指令。"));
    connect(m_editor, &QPlainTextEdit::textChanged, this, [this]() {
        if (m_loading) {
            return;
        }
        m_saveButton->setEnabled(m_editor->toPlainText() != m_savedSoul);
    });
    m_mainLayout->addWidget(m_editor, 1);

    QLabel *hint = new QLabel(tr("此文件会在每次会话时重新读取，可用于定义智能体的人格、沟通风格和长期指令。"), this);
    hint->setObjectName(QStringLiteral("hintLabel"));
    hint->setWordWrap(true);
    m_mainLayout->addWidget(hint);
}

void PersonaSettingsWidget::resetPersona() {
    QWidget *ownerWindow = window();
    kdk::KMessageBox confirm(ownerWindow);
    confirm.setIcon(kdk::KMessageBox::Question);
    confirm.setWindowTitle(tr("重置个性"));
    confirm.setText(tr("确定要将 SOUL.md 重置为默认内容吗？"));
    QPushButton *resetButton = confirm.addButton(tr("重置"), kdk::KMessageBox::AcceptRole);
    confirm.addButton(tr("取消"), kdk::KMessageBox::RejectRole);
    QTimer::singleShot(0, &confirm, [&confirm, ownerWindow]() {
        if (!ownerWindow) {
            return;
        }
        confirm.move(ownerWindow->frameGeometry().center() -
                     QPoint(confirm.width() / 2, confirm.height() / 2));
        confirm.raise();
    });
    confirm.exec();
    if (confirm.clickedButton() != resetButton) {
        return;
    }
    m_editor->setPlainText(defaultSoul());
    m_resetButton->setEnabled(false);
    m_resetButton->setText(tr("已重置"));
    QTimer::singleShot(1000, this, [this]() {
        m_resetButton->setText(tr("重置"));
        m_resetButton->setEnabled(true);
    });
}

bool PersonaSettingsWidget::savePersona(bool showConfirmation) {
    const QString content = m_editor->toPlainText();
    if (!writeSoul(content)) {
        kdk::KMessageBox::warning(this, tr("保存失败"), tr("无法保存个性设置。"));
        return false;
    }

    m_savedSoul = content;
    m_saveButton->setEnabled(false);
    if (showConfirmation) {
        QWidget *ownerWindow = window();
        kdk::KMessageBox savedMessage(ownerWindow);
        savedMessage.setIcon(kdk::KMessageBox::Information);
        savedMessage.setWindowTitle(tr("已保存"));
        savedMessage.setText(tr("个性设置已保存。"));
        QPushButton *savedCloseButton = savedMessage.addButton(kdk::KMessageBox::Close);
        savedCloseButton->setText(tr("关闭"));
        QTimer::singleShot(0, &savedMessage, [&savedMessage, ownerWindow]() {
            if (!ownerWindow) {
                return;
            }
            savedMessage.move(ownerWindow->frameGeometry().center() -
                              QPoint(savedMessage.width() / 2, savedMessage.height() / 2));
            savedMessage.raise();
        });
        QTimer::singleShot(1500, &savedMessage, &kdk::KMessageBox::accept);
        savedMessage.exec();
    }
    return true;
}

QString PersonaSettingsWidget::soulFilePath() const {
    return QStringLiteral("SOUL.md");
}

QString PersonaSettingsWidget::runtimeHome() const {
    return QString();
}

QString PersonaSettingsWidget::readSoul() const {
    const QString content = ApiService::instance().getPersonaSync();
    return content.isEmpty() ? defaultSoul() : content;
}

bool PersonaSettingsWidget::writeSoul(const QString &content) const {
    return ApiService::instance().putPersonaSync(content);
}

QString PersonaSettingsWidget::defaultSoul() const {
    return QStringLiteral(
        "You are KylinAgent, an intelligent AI assistant created by NUDT, HIT (Shenzhen), and KylinSoft, and an important component of the openKylin AgentOS ecosystem. You are helpful, knowledgeable, and direct. You assist users with a wide range of tasks including answering questions, writing and editing code, analyzing information, creative work, and executing actions via your tools. You communicate clearly, admit uncertainty when appropriate, and prioritize being genuinely useful over being verbose unless otherwise directed below. Be targeted and efficient in your exploration and investigations. For all tasks other than translation, respond and provide feedback only in the language used by the user.\n");
}

void PersonaSettingsWidget::loadSettings() {
    const QString content = readSoul();
    m_loading = true;
    m_editor->setPlainText(content);
    m_loading = false;
    m_savedSoul = content;
    m_saveButton->setEnabled(false);
}

void PersonaSettingsWidget::saveSettings() {
    if (m_editor->toPlainText() != m_savedSoul) {
        savePersona(false);
    }
}
