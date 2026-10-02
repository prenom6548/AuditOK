#include "ui/usagesettingswidget.h"
#include "services/apiservice.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

UsageSettingsWidget::UsageSettingsWidget(QWidget *parent)
    : QWidget(parent),
      m_mainLayout(new QVBoxLayout(this)),
      m_refreshButton(nullptr),
      m_statsLabel(nullptr) {
    createUsageGroup();
    refreshUsage();
}

void UsageSettingsWidget::createUsageGroup() {
    setObjectName(QStringLiteral("usageSettingsWidget"));
    setMinimumWidth(520);
    m_mainLayout->setContentsMargins(28, 24, 28, 28);
    m_mainLayout->setSpacing(14);

    setStyleSheet(R"(
        #usageSettingsWidget { background: palette(window); color: palette(window-text); }
        QLabel#usageTitle { font-size: 22px; font-weight: 700; color: palette(window-text); }
        QLabel#usageSubtitle {
            font-size: 14px; color: palette(placeholder-text); margin-bottom: 8px;
        }
        QLabel#usageStats {
            background: palette(base); border: 1px solid palette(mid); border-radius: 8px;
            padding: 20px; font-size: 13px; color: palette(text);
        }
        QPushButton { background: palette(button); border: 1px solid palette(mid);
                      border-radius: 6px; padding: 6px 14px; font-weight: 600;
                      color: palette(button-text); }
        QPushButton:hover { background: palette(mid); }
        QPushButton:disabled { background: palette(midlight); color: #8a8a8a; }
    )");

    QLabel *title = new QLabel(tr("用量统计"), this);
    title->setObjectName(QStringLiteral("usageTitle"));
    m_mainLayout->addWidget(title);

    QLabel *subtitle = new QLabel(tr("查看 Token 使用量和会话统计"), this);
    subtitle->setObjectName(QStringLiteral("usageSubtitle"));
    m_mainLayout->addWidget(subtitle);

    QHBoxLayout *toolbar = new QHBoxLayout();
    m_refreshButton = new QPushButton(tr("↻ 刷新"), this);
    connect(m_refreshButton, &QPushButton::clicked,
            this, &UsageSettingsWidget::refreshUsageWithFeedback);
    toolbar->addWidget(m_refreshButton);
    toolbar->addStretch();
    m_mainLayout->addLayout(toolbar);

    m_statsLabel = new QLabel(this);
    m_statsLabel->setObjectName(QStringLiteral("usageStats"));
    m_statsLabel->setWordWrap(true);
    m_statsLabel->setTextFormat(Qt::RichText);
    m_statsLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_mainLayout->addWidget(m_statsLabel, 1);
}

void UsageSettingsWidget::refreshUsage() {
    const QMap<QString, QVariant> usage = ApiService::instance().getUsageSync();
    const qlonglong inputTokens = usage.value(QStringLiteral("input_tokens")).toLongLong();
    const qlonglong outputTokens = usage.value(QStringLiteral("output_tokens")).toLongLong();
    const QLocale locale;
    const auto row = [](const QString &label, const QString &value) {
        return QStringLiteral(
            "<tr><td width='65%' style='padding:6px 8px 6px 0;'>%1</td>"
            "<td align='right' style='padding:6px 0;'><b>%2</b></td></tr>")
            .arg(label.toHtmlEscaped(), value.toHtmlEscaped());
    };

    QString html = QStringLiteral("<h3>%1</h3><table width='100%' cellspacing='0'>")
        .arg(tr("服务端统计").toHtmlEscaped());
    html += row(tr("总会话数"),
                QString::number(usage.value(QStringLiteral("sessions")).toInt()));
    html += row(tr("总消息数"),
                QString::number(usage.value(QStringLiteral("messages")).toInt()));
    html += row(tr("输入 Token"), locale.toString(inputTokens));
    html += row(tr("输出 Token"), locale.toString(outputTokens));
    html += row(tr("Token 总计"), locale.toString(inputTokens + outputTokens));
    html += QStringLiteral("</table><br><small>%1</small>")
        .arg(tr("数据来自 Kylin-Agent-Runtime 服务。").toHtmlEscaped());

    m_statsLabel->setText(html);
}

void UsageSettingsWidget::refreshUsageWithFeedback() {
    if (!m_refreshButton || !m_refreshButton->isEnabled()) {
        return;
    }
    m_refreshButton->setEnabled(false);
    m_refreshButton->setText(tr("↻ 刷新中…"));
    QTimer::singleShot(0, this, [this]() {
        refreshUsage();
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

void UsageSettingsWidget::loadSettings() { refreshUsage(); }
void UsageSettingsWidget::saveSettings() {}
