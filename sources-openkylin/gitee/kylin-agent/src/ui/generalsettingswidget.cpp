#include "ui/generalsettingswidget.h"
#include "utils/thememanager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <kmessagebox.h>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSignalBlocker>
#include <QStandardPaths>
#include <QTextStream>

namespace {

QString autoStartDesktopPath()
{
    const QString configHome = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    return QDir(configHome).filePath(QStringLiteral("autostart/kylin-agent.desktop"));
}

QString desktopExecQuote(QString value)
{
    value.replace(QLatin1Char('\\'), QStringLiteral("\\\\"));
    value.replace(QLatin1Char('"'), QStringLiteral("\\\""));
    value.replace(QLatin1Char('%'), QStringLiteral("%%"));
    return QLatin1Char('"') + value + QLatin1Char('"');
}

bool setAutoStartEnabled(bool enabled)
{
    const QString path = autoStartDesktopPath();
    if (!enabled) {
        return !QFileInfo::exists(path) || QFile::remove(path);
    }

    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out << "[Desktop Entry]\n"
           "Type=Application\n"
           "Name=KylinAgent\n"
           "Comment=Start KylinAgent and prepare the startup greeting\n"
           "Exec="
        << desktopExecQuote(QCoreApplication::applicationFilePath())
        << "\nIcon=kylin-agent\n"
           "Terminal=false\n"
           "X-GNOME-Autostart-enabled=true\n";
    return file.commit();
}

QString runtimeConfigPath()
{
    const QByteArray agentRuntimeHome = qgetenv("AGENT_RUNTIME_HOME");
    if (!agentRuntimeHome.isEmpty()) {
        return QDir(QString::fromLocal8Bit(agentRuntimeHome)).filePath(QStringLiteral("config.yaml"));
    }

    const QByteArray envHome = qgetenv("HERMES_HOME");
    const QString home = envHome.isEmpty()
        ? QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).filePath(QStringLiteral(".kylin-agent-runtime"))
        : QString::fromLocal8Bit(envHome);
    return QDir(home).filePath(QStringLiteral("config.yaml"));
}

QStringList readRuntimeConfigLines()
{
    QFile file(runtimeConfigPath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    QStringList lines;
    QTextStream in(&file);
    while (!in.atEnd()) {
        lines.append(in.readLine());
    }
    return lines;
}

bool writeRuntimeConfigLines(const QStringList &lines)
{
    const QString path = runtimeConfigPath();
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream out(&file);
    out << lines.join(QLatin1Char('\n'));
    if (!lines.isEmpty()) {
        out << '\n';
    }
    return file.commit();
}

bool readTopLevelBool(const QString &key, bool defaultValue)
{
    const QRegularExpression re(QStringLiteral("^%1:\\s*(.+?)\\s*(?:#.*)?$")
                                    .arg(QRegularExpression::escape(key)));
    for (const QString &line : readRuntimeConfigLines()) {
        if (line.startsWith(QLatin1Char(' ')) || line.startsWith(QLatin1Char('\t'))) {
            continue;
        }
        const QRegularExpressionMatch match = re.match(line);
        if (!match.hasMatch()) {
            continue;
        }
        QString value = match.captured(1).trimmed().toLower();
        if ((value.startsWith(QLatin1Char('"')) && value.endsWith(QLatin1Char('"'))) ||
            (value.startsWith(QLatin1Char('\'')) && value.endsWith(QLatin1Char('\'')))) {
            value = value.mid(1, value.size() - 2);
        }
        if (value == QStringLiteral("true") || value == QStringLiteral("yes") ||
            value == QStringLiteral("on") || value == QStringLiteral("1")) {
            return true;
        }
        if (value == QStringLiteral("false") || value == QStringLiteral("no") ||
            value == QStringLiteral("off") || value == QStringLiteral("0")) {
            return false;
        }
    }
    return defaultValue;
}

bool readNestedBool(const QString &section, const QString &key, bool defaultValue, bool *found = nullptr)
{
    if (found) {
        *found = false;
    }

    const QStringList lines = readRuntimeConfigLines();
    const QRegularExpression sectionRe(QStringLiteral("^%1:\\s*(?:#.*)?$")
                                           .arg(QRegularExpression::escape(section)));
    const QRegularExpression keyRe(QStringLiteral("^\\s+%1:\\s*(.+?)\\s*(?:#.*)?$")
                                       .arg(QRegularExpression::escape(key)));
    bool inSection = false;
    for (const QString &line : lines) {
        if (!inSection) {
            if (sectionRe.match(line).hasMatch()) {
                inSection = true;
            }
            continue;
        }

        if (!line.trimmed().isEmpty() &&
            !line.startsWith(QLatin1Char(' ')) &&
            !line.startsWith(QLatin1Char('\t'))) {
            break;
        }

        const QRegularExpressionMatch match = keyRe.match(line);
        if (!match.hasMatch()) {
            continue;
        }
        if (found) {
            *found = true;
        }
        QString value = match.captured(1).trimmed().toLower();
        if ((value.startsWith(QLatin1Char('"')) && value.endsWith(QLatin1Char('"'))) ||
            (value.startsWith(QLatin1Char('\'')) && value.endsWith(QLatin1Char('\'')))) {
            value = value.mid(1, value.size() - 2);
        }
        if (value == QStringLiteral("true") || value == QStringLiteral("yes") ||
            value == QStringLiteral("on") || value == QStringLiteral("1")) {
            return true;
        }
        if (value == QStringLiteral("false") || value == QStringLiteral("no") ||
            value == QStringLiteral("off") || value == QStringLiteral("0")) {
            return false;
        }
        return defaultValue;
    }
    return defaultValue;
}

bool writeTopLevelBool(const QString &key, bool value)
{
    QStringList lines = readRuntimeConfigLines();
    const QRegularExpression re(QStringLiteral("^%1\\s*:.*$")
                                    .arg(QRegularExpression::escape(key)));
    const QString replacement = QStringLiteral("%1: %2")
                                    .arg(key, value ? QStringLiteral("true") : QStringLiteral("false"));
    for (int i = 0; i < lines.size(); ++i) {
        const QString line = lines.at(i);
        if (line.startsWith(QLatin1Char(' ')) || line.startsWith(QLatin1Char('\t'))) {
            continue;
        }
        if (re.match(line).hasMatch()) {
            lines[i] = replacement;
            return writeRuntimeConfigLines(lines);
        }
    }
    if (!lines.isEmpty() && !lines.last().trimmed().isEmpty()) {
        lines.append(QString());
    }
    lines.append(replacement);
    return writeRuntimeConfigLines(lines);
}

bool writeNestedBool(const QString &section, const QString &key, bool value)
{
    QStringList lines = readRuntimeConfigLines();
    const QString replacement = QStringLiteral("  %1: %2")
                                    .arg(key, value ? QStringLiteral("true") : QStringLiteral("false"));
    const QRegularExpression sectionRe(QStringLiteral("^%1\\s*:.*$")
                                           .arg(QRegularExpression::escape(section)));
    const QRegularExpression exactSectionRe(QStringLiteral("^%1:\\s*(?:#.*)?$")
                                                .arg(QRegularExpression::escape(section)));
    const QRegularExpression nestedKeyRe(QStringLiteral("^\\s+%1\\s*:.*$")
                                             .arg(QRegularExpression::escape(key)));
    const QRegularExpression legacyTopLevelRe(QStringLiteral("^%1\\s*:.*$")
                                                  .arg(QRegularExpression::escape(key)));

    for (int i = lines.size() - 1; i >= 0; --i) {
        const QString line = lines.at(i);
        if (!line.startsWith(QLatin1Char(' ')) &&
            !line.startsWith(QLatin1Char('\t')) &&
            legacyTopLevelRe.match(line).hasMatch()) {
            lines.removeAt(i);
        }
    }

    int sectionIndex = -1;
    int sectionEnd = lines.size();
    for (int i = 0; i < lines.size(); ++i) {
        if (!sectionRe.match(lines.at(i)).hasMatch()) {
            continue;
        }
        sectionIndex = i;
        if (!exactSectionRe.match(lines.at(i)).hasMatch()) {
            lines[i] = section + QLatin1Char(':');
        }
        sectionEnd = lines.size();
        for (int j = i + 1; j < lines.size(); ++j) {
            const QString line = lines.at(j);
            if (!line.trimmed().isEmpty() &&
                !line.startsWith(QLatin1Char(' ')) &&
                !line.startsWith(QLatin1Char('\t'))) {
                sectionEnd = j;
                break;
            }
        }
        break;
    }

    if (sectionIndex < 0) {
        if (!lines.isEmpty() && !lines.last().trimmed().isEmpty()) {
            lines.append(QString());
        }
        lines.append(section + QLatin1Char(':'));
        lines.append(replacement);
        return writeRuntimeConfigLines(lines);
    }

    for (int i = sectionIndex + 1; i < sectionEnd; ++i) {
        if (nestedKeyRe.match(lines.at(i)).hasMatch()) {
            lines[i] = replacement;
            return writeRuntimeConfigLines(lines);
        }
    }

    lines.insert(sectionIndex + 1, replacement);
    return writeRuntimeConfigLines(lines);
}

} // namespace

GeneralSettingsWidget::GeneralSettingsWidget(QWidget *parent) 
    : QWidget(parent), 
      m_formLayout(new QFormLayout(this))
{
    setLayout(m_formLayout);
    createGeneralGroup();
    loadSettings();
}

void GeneralSettingsWidget::createGeneralGroup() {
    // Theme selection
    m_themeCombo = new QComboBox(this);
    m_themeCombo->addItems({tr("跟随系统"), tr("浅色"), tr("深色")});
    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [](int index) {
        ThemeManager::instance().setThemeMode(static_cast<ThemeManager::ThemeMode>(index));
    });
    
    // General options
    m_enableAutoStartCheckBox = new QCheckBox(tr("开机自启动"), this);
    m_enableAutoStartCheckBox->setChecked(false);
    connect(m_enableAutoStartCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        if (!setAutoStartEnabled(checked)) {
            QSignalBlocker blocker(m_enableAutoStartCheckBox);
            m_enableAutoStartCheckBox->setChecked(!checked);
            kdk::KMessageBox::warning(this, tr("保存失败"), tr("无法更新开机自启动配置。"));
            return;
        }
        QSettings settings;
        settings.setValue(QStringLiteral("General/AutoStart"), checked);
        settings.sync();
    });

    m_startupGreetingCheckBox = new QCheckBox(tr("启动时主动问候"), this);
    m_startupGreetingCheckBox->setChecked(true);
    connect(m_startupGreetingCheckBox, &QCheckBox::toggled, this, [](bool checked) {
        QSettings settings;
        settings.setValue(QStringLiteral("General/StartupGreeting"), checked);
    });

    m_enableSystemTrayCheckBox = new QCheckBox(tr("最小化到系统托盘"), this);
    m_enableSystemTrayCheckBox->setChecked(true);

    m_enableDelegationModeCheckBox = new QCheckBox(tr("启用委派模式"), this);
    m_enableDelegationModeCheckBox->setChecked(true);
    connect(m_enableDelegationModeCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        if (!writeNestedBool(QStringLiteral("delegation"), QStringLiteral("mainline_branch_enabled"), checked)) {
            QSignalBlocker blocker(m_enableDelegationModeCheckBox);
            m_enableDelegationModeCheckBox->setChecked(!checked);
            kdk::KMessageBox::warning(this, tr("保存失败"), tr("无法写入运行时 config.yaml。"));
        }
    });

    m_showDetailsPreviewButtonCheckBox = new QCheckBox(tr("显示详情预览按钮"), this);
    m_showDetailsPreviewButtonCheckBox->setChecked(true);
    connect(m_showDetailsPreviewButtonCheckBox, &QCheckBox::toggled, this, [](bool checked) {
        QSettings settings;
        settings.setValue(QStringLiteral("General/ShowDetailsPreviewButton"), checked);
    });

    m_showSessionExportButtonCheckBox = new QCheckBox(tr("显示会话导出按钮"), this);
    m_showSessionExportButtonCheckBox->setChecked(true);
    connect(m_showSessionExportButtonCheckBox, &QCheckBox::toggled, this, [](bool checked) {
        QSettings settings;
        settings.setValue(QStringLiteral("General/ShowSessionExportButton"), checked);
    });

    m_showSessionTokenTotalCheckBox = new QCheckBox(tr("显示会话Token总消耗"), this);
    m_showSessionTokenTotalCheckBox->setChecked(true);
    connect(m_showSessionTokenTotalCheckBox, &QCheckBox::toggled, this, [](bool checked) {
        QSettings settings;
        settings.setValue(QStringLiteral("General/ShowSessionTokenTotal"), checked);
    });

    m_showSessionContextLengthCheckBox = new QCheckBox(tr("显示会话上下文长度"), this);
    m_showSessionContextLengthCheckBox->setChecked(true);
    connect(m_showSessionContextLengthCheckBox, &QCheckBox::toggled, this, [](bool checked) {
        QSettings settings;
        settings.setValue(QStringLiteral("General/ShowSessionContextLength"), checked);
    });
    
    m_enableVoiceWakeCheckBox = new QCheckBox(tr("启用语音唤醒"), this);
    m_enableVoiceWakeCheckBox->setChecked(true);

    m_autoSendAfterVoiceRecognitionCheckBox = new QCheckBox(tr("语音识别后自动结束后自动发送"), this);
    m_autoSendAfterVoiceRecognitionCheckBox->setChecked(true);

    m_voiceToneCombo = new QComboBox(this);
    m_voiceToneCombo->addItems({
        tr("清晰简练-女"),
        tr("温暖沉稳-男"),
        tr("柔和清晰-女"),
        tr("温暖沉稳-女"),
        tr("温暖清晰-女")
    });

    m_autoVoiceBroadcastCheckBox = new QCheckBox(tr("自动播报"), this);
    m_autoVoiceBroadcastCheckBox->setChecked(true);
    
    // Add to form layout
    m_formLayout->addRow(tr("主题："), m_themeCombo);
    m_formLayout->addRow(m_enableAutoStartCheckBox);
    m_formLayout->addRow(m_startupGreetingCheckBox);
    m_formLayout->addRow(m_enableSystemTrayCheckBox);
    m_formLayout->addRow(m_enableDelegationModeCheckBox);
    m_formLayout->addRow(m_showDetailsPreviewButtonCheckBox);
    m_formLayout->addRow(m_showSessionExportButtonCheckBox);
    m_formLayout->addRow(m_showSessionTokenTotalCheckBox);
    m_formLayout->addRow(m_showSessionContextLengthCheckBox);
    m_formLayout->addRow(m_enableVoiceWakeCheckBox);
    m_formLayout->addRow(m_autoSendAfterVoiceRecognitionCheckBox);
    m_formLayout->addRow(tr("启用语音播报音色："), m_voiceToneCombo);
    m_formLayout->addRow(m_autoVoiceBroadcastCheckBox);
    
    // Set default values
    m_voiceToneCombo->setCurrentIndex(0);
}

void GeneralSettingsWidget::loadSettings() {
    // Implementation for loading general settings from configuration
    QSettings settings;
    const int themeIndex = settings.value("General/Theme", 0).toInt();
    const int normalizedThemeIndex = themeIndex >= 0 && themeIndex <= 2 ? themeIndex : 0;
    {
        QSignalBlocker blocker(m_themeCombo);
        m_themeCombo->setCurrentIndex(normalizedThemeIndex);
    }
    ThemeManager::instance().setThemeMode(static_cast<ThemeManager::ThemeMode>(normalizedThemeIndex));
    const bool autoStartEnabled = settings.value("General/AutoStart", false).toBool();
    {
        QSignalBlocker blocker(m_enableAutoStartCheckBox);
        m_enableAutoStartCheckBox->setChecked(autoStartEnabled);
    }
    if (!setAutoStartEnabled(autoStartEnabled)) {
        QSignalBlocker blocker(m_enableAutoStartCheckBox);
        m_enableAutoStartCheckBox->setChecked(false);
        settings.setValue(QStringLiteral("General/AutoStart"), false);
        kdk::KMessageBox::warning(this, tr("保存失败"), tr("无法同步开机自启动配置。"));
    }
    {
        QSignalBlocker blocker(m_startupGreetingCheckBox);
        m_startupGreetingCheckBox->setChecked(
            settings.value(QStringLiteral("General/StartupGreeting"), true).toBool());
    }
    m_enableSystemTrayCheckBox->setChecked(settings.value("General/SystemTray", true).toBool());
    {
        QSignalBlocker blocker(m_enableDelegationModeCheckBox);
        bool foundDelegationSetting = false;
        const bool delegationEnabled = readNestedBool(QStringLiteral("delegation"),
                                                      QStringLiteral("mainline_branch_enabled"),
                                                      true,
                                                      &foundDelegationSetting);
        m_enableDelegationModeCheckBox->setChecked(foundDelegationSetting
            ? delegationEnabled
            : readTopLevelBool(QStringLiteral("mainline_branch_enabled"), true));
    }
    {
        QSignalBlocker blocker(m_showDetailsPreviewButtonCheckBox);
        m_showDetailsPreviewButtonCheckBox->setChecked(
            settings.value("General/ShowDetailsPreviewButton", true).toBool());
    }
    {
        QSignalBlocker blocker(m_showSessionExportButtonCheckBox);
        m_showSessionExportButtonCheckBox->setChecked(
            settings.value("General/ShowSessionExportButton", true).toBool());
    }
    {
        QSignalBlocker blocker(m_showSessionTokenTotalCheckBox);
        m_showSessionTokenTotalCheckBox->setChecked(
            settings.value("General/ShowSessionTokenTotal", true).toBool());
    }
    {
        QSignalBlocker blocker(m_showSessionContextLengthCheckBox);
        m_showSessionContextLengthCheckBox->setChecked(
            settings.value("General/ShowSessionContextLength", true).toBool());
    }
    m_enableVoiceWakeCheckBox->setChecked(settings.value("General/VoiceWakeEnabled", true).toBool());
    m_autoSendAfterVoiceRecognitionCheckBox->setChecked(settings.value("General/VoiceAutoSend", true).toBool());
    m_voiceToneCombo->setCurrentIndex(settings.value("General/VoiceTone", 0).toInt());
    m_autoVoiceBroadcastCheckBox->setChecked(settings.value("General/AutoVoiceBroadcast", true).toBool());
}

void GeneralSettingsWidget::saveSettings() {
    // Implementation for saving general settings to configuration
    QSettings settings;
    settings.setValue("General/Theme", m_themeCombo->currentIndex());
    settings.setValue("General/AutoStart", m_enableAutoStartCheckBox->isChecked());
    settings.setValue("General/StartupGreeting", m_startupGreetingCheckBox->isChecked());
    settings.setValue("General/SystemTray", m_enableSystemTrayCheckBox->isChecked());
    settings.setValue("General/ShowDetailsPreviewButton", m_showDetailsPreviewButtonCheckBox->isChecked());
    settings.setValue("General/ShowSessionExportButton", m_showSessionExportButtonCheckBox->isChecked());
    settings.setValue("General/ShowSessionTokenTotal", m_showSessionTokenTotalCheckBox->isChecked());
    settings.setValue("General/ShowSessionContextLength", m_showSessionContextLengthCheckBox->isChecked());
    writeNestedBool(QStringLiteral("delegation"), QStringLiteral("mainline_branch_enabled"),
                    m_enableDelegationModeCheckBox->isChecked());
    settings.setValue("General/VoiceWakeEnabled", m_enableVoiceWakeCheckBox->isChecked());
    settings.setValue("General/VoiceAutoSend", m_autoSendAfterVoiceRecognitionCheckBox->isChecked());
    settings.setValue("General/VoiceTone", m_voiceToneCombo->currentIndex());
    settings.setValue("General/AutoVoiceBroadcast", m_autoVoiceBroadcastCheckBox->isChecked());
    settings.sync();
}
