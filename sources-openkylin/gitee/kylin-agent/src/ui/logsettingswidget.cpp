#include "ui/logsettingswidget.h"
#include "services/apiservice.h"
#include "utils/thememanager.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QScrollBar>
#include <QSplitter>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>

LogSettingsWidget::LogSettingsWidget(QWidget *parent)
    : QWidget(parent),
      m_mainLayout(new QVBoxLayout(this)),
      m_refreshButton(nullptr),
      m_fileList(nullptr),
      m_logContent(nullptr) {
    createLogGroup();
    refreshLogList();
}

void LogSettingsWidget::createLogGroup() {
    setObjectName(QStringLiteral("logSettingsWidget"));
    setMinimumWidth(520);
    m_mainLayout->setContentsMargins(28, 24, 28, 28);
    m_mainLayout->setSpacing(14);

    setStyleSheet(R"(
        #logSettingsWidget { background: palette(window); color: palette(window-text); }
        QLabel#logTitle { font-size: 22px; font-weight: 700; color: palette(window-text); }
        QLabel#logSubtitle {
            font-size: 14px; color: palette(placeholder-text); margin-bottom: 8px;
        }
        QTextEdit {
            background: palette(base); border: 1px solid palette(mid); border-radius: 6px;
            font-family: "Monospace"; font-size: 12px; color: palette(text);
            selection-background-color: palette(highlight);
            selection-color: palette(highlighted-text);
        }
        QPushButton { background: palette(button); border: 1px solid palette(mid);
                      border-radius: 6px; padding: 6px 14px; font-weight: 600;
                      color: palette(button-text); }
        QPushButton:hover { background: palette(mid); }
        QPushButton:disabled { background: palette(midlight); color: #8a8a8a; }
    )");

    QLabel *title = new QLabel(tr("日志"), this);
    title->setObjectName(QStringLiteral("logTitle"));
    m_mainLayout->addWidget(title);

    QLabel *subtitle = new QLabel(tr("查看 Kylin-Agent-Runtime 服务运行日志"), this);
    subtitle->setObjectName(QStringLiteral("logSubtitle"));
    m_mainLayout->addWidget(subtitle);

    QHBoxLayout *toolbar = new QHBoxLayout();
    m_refreshButton = new QPushButton(tr("↻ 刷新"), this);
    connect(m_refreshButton, &QPushButton::clicked,
            this, &LogSettingsWidget::refreshLogListWithFeedback);
    toolbar->addWidget(m_refreshButton);
    toolbar->addStretch();
    m_mainLayout->addLayout(toolbar);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(1);
    splitter->setStyleSheet("QSplitter::handle { background: palette(mid); }");

    m_fileList = new QListWidget(this);
    m_fileList->setObjectName(QStringLiteral("logFileList"));
    m_fileList->setMinimumWidth(200);
    m_fileList->setMaximumWidth(260);
    m_fileList->setSpacing(2);
    m_fileList->setAlternatingRowColors(true);
    m_fileList->setProperty("darkTheme", ThemeManager::instance().isDark());
    m_fileList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_fileList->setStyleSheet(R"(
        QListWidget#logFileList {
            background: palette(base);
            color: palette(text);
            border: 1px solid palette(mid);
            border-right: none;
            border-radius: 6px 0 0 6px;
            outline: none;
            padding: 4px;
            alternate-background-color: rgba(127, 127, 127, 12);
        }
        QListWidget#logFileList::item {
            color: palette(text);
            background: transparent;
            border: 1px solid transparent;
            border-radius: 6px;
            padding: 9px 10px;
        }
        QListWidget#logFileList::item:alternate {
            background: rgba(127, 127, 127, 10);
        }
        QListWidget#logFileList::item:hover {
            background: #eef7ff;
            border-color: #cfe8fb;
        }
        QListWidget#logFileList::item:selected {
            background: #e3f2fd;
            color: #1976d2;
            border-color: #b9dcfb;
        }
        QListWidget#logFileList[darkTheme="true"]::item:hover {
            background: #242b35;
            border-color: #3a4654;
        }
        QListWidget#logFileList[darkTheme="true"]::item:selected {
            background: #263a54;
            color: #8ab4f8;
            border-color: #3e5778;
        }
    )");

    m_logContent = new QTextEdit(this);
    m_logContent->setReadOnly(true);
    m_logContent->setLineWrapMode(QTextEdit::NoWrap);
    m_logContent->setStyleSheet("QTextEdit { border-radius: 0 6px 6px 0; }");

    splitter->addWidget(m_fileList);
    splitter->addWidget(m_logContent);
    splitter->setSizes({200, 380});
    m_mainLayout->addWidget(splitter, 1);

    connect(m_fileList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        if (item) {
            const QString fileName = item->data(Qt::UserRole).toString();
            if (!fileName.isEmpty()) {
                loadLogContent(fileName);
            }
        }
    });
}

void LogSettingsWidget::refreshLogList() {
    const QString selectedFile = m_fileList->currentItem()
        ? m_fileList->currentItem()->data(Qt::UserRole).toString()
        : QString();
    m_fileList->clear();

    const auto files = ApiService::instance().getLogsSync();
    if (files.isEmpty()) {
        QListWidgetItem *emptyItem = new QListWidgetItem(tr("暂无日志文件"), m_fileList);
        emptyItem->setFlags(Qt::NoItemFlags);
        emptyItem->setTextAlignment(Qt::AlignCenter);
        m_logContent->clear();
        return;
    }

    QListWidgetItem *itemToSelect = nullptr;
    for (const auto &fi : files) {
        qint64 kb = fi.value(QStringLiteral("size")).toLongLong() / 1024;
        QString sizeStr = kb > 1024 ? QString::number(kb / 1024) + " MB"
                                    : QString::number(kb) + " KB";
        const QString fileName = fi.value(QStringLiteral("name")).toString();
        QListWidgetItem *item = new QListWidgetItem(
            QStringLiteral("%1\n%2").arg(fileName, sizeStr), m_fileList);
        item->setData(Qt::UserRole, fileName);
        item->setToolTip(fileName);
        item->setSizeHint(QSize(0, 54));
        if (!itemToSelect || fileName == selectedFile) {
            itemToSelect = item;
        }
    }
    if (itemToSelect) {
        m_fileList->setCurrentItem(itemToSelect);
        loadLogContent(itemToSelect->data(Qt::UserRole).toString());
    }
}

void LogSettingsWidget::refreshLogListWithFeedback() {
    if (!m_refreshButton || !m_refreshButton->isEnabled()) {
        return;
    }
    m_refreshButton->setEnabled(false);
    m_refreshButton->setText(tr("↻ 刷新中…"));
    QTimer::singleShot(0, this, [this]() {
        refreshLogList();
        m_refreshButton->setEnabled(false);
        m_refreshButton->setText(tr("✓ 已刷新"));
        QTimer::singleShot(1000, this, [this]() {
            if (m_refreshButton) {
                m_refreshButton->setText(tr("↻ 刷新"));
                m_refreshButton->setEnabled(true);
            }
        });
    });
}

void LogSettingsWidget::loadLogContent(const QString &filePath) {
    QString content = ApiService::instance().getLogContentSync(filePath);
    if (content.isEmpty()) {
        m_logContent->setPlainText(tr("无法读取：%1").arg(filePath));
        return;
    }
    if (content.size() > 200 * 1024) {
        content = tr("... (仅显示最后 200 KB)\n\n") + content.right(200 * 1024);
    }

    m_logContent->setPlainText(content);
    // Scroll to bottom
    QScrollBar *sb = m_logContent->verticalScrollBar();
    sb->setValue(sb->maximum());
}

QString LogSettingsWidget::runtimeHome() const {
    return QString();
}

void LogSettingsWidget::loadSettings() { refreshLogList(); }
void LogSettingsWidget::saveSettings() {}
