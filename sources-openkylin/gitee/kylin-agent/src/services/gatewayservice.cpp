#include "services/gatewayservice.h"

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QLabel>
#include <kmessagebox.h>
#include <kdialog.h>
#include <QPlainTextEdit>
#include <QProcess>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QPixmap>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTcpSocket>
#include <QTextStream>
#include <QThread>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QVersionNumber>

namespace {

constexpr int kApiServerPort = 8642;
constexpr int kGatewayStatusTimeoutMs = 5000;
constexpr int kGatewayStopTimeoutMs = 5000;
constexpr int kSystemctlTimeoutMs = 30000;
constexpr int kNoInstallTimeoutMs = 0;
const QString kGatewayServiceName = QStringLiteral("kylin-agent-runtime-gateway.service");
const QString kNotifierServiceName = QStringLiteral("kylin-agent-runtime-notifier.service");

QString shellQuote(QString value)
{
    value.replace(QLatin1Char('\''), QStringLiteral("'\\''"));
    return QStringLiteral("'") + value + QStringLiteral("'");
}

QString systemdQuote(QString value)
{
    value.replace(QLatin1Char('\\'), QStringLiteral("\\\\"));
    value.replace(QLatin1Char('"'), QStringLiteral("\\\""));
    value.replace(QLatin1Char('%'), QStringLiteral("%%"));
    return QStringLiteral("\"") + value + QStringLiteral("\"");
}

QString readTextFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    return QString::fromUtf8(file.readAll());
}

bool writeTextFile(const QString &path, const QString &content)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }
    if (file.write(content.toUtf8()) < 0) {
        return false;
    }
    return file.commit();
}

QVersionNumber parseVersion(QString version)
{
    version = version.trimmed();
    if (version.startsWith(QLatin1Char('v'), Qt::CaseInsensitive)) {
        version.remove(0, 1);
    }

    int end = 0;
    while (end < version.size()) {
        const QChar ch = version.at(end);
        if (!ch.isDigit() && ch != QLatin1Char('.')) {
            break;
        }
        ++end;
    }

    return QVersionNumber::fromString(version.left(end));
}

bool isVersionOlderThan(const QString &installedVersion, const QString &expectedVersion)
{
    const QVersionNumber installed = parseVersion(installedVersion);
    const QVersionNumber expected = parseVersion(expectedVersion);
    if (installed.isNull() || expected.isNull()) {
        return installedVersion.trimmed() != expectedVersion.trimmed();
    }

    return QVersionNumber::compare(installed, expected) < 0;
}

bool isVersionAtLeast(const QString &installedVersion, const QString &expectedVersion)
{
    return !installedVersion.trimmed().isEmpty() && !isVersionOlderThan(installedVersion, expectedVersion);
}

QString defaultSoulMd()
{
    return QStringLiteral(
        "You are KylinAgent, an intelligent AI assistant created by NUDT, HIT (Shenzhen), "
        "and KylinSoft, and an important component of the openKylin AgentOS ecosystem. "
        "You are helpful, knowledgeable, and direct. You assist users with a wide range "
        "of tasks including answering questions, writing and editing code, analyzing "
        "information, creative work, and executing actions via your tools. You communicate "
        "clearly, admit uncertainty when appropriate, and prioritize being genuinely useful "
        "over being verbose unless otherwise directed below. Be targeted and efficient in "
        "your exploration and investigations. For all tasks other than translation, respond "
        "and provide feedback only in the language used by the user.\n");
}

bool showMessageDialog(QWidget *parent,
                       const QString &title,
                       const QString &text,
                       const QString &acceptText,
                       const QString &cancelText = QString())
{
    kdk::KMessageBox dialog(parent);
    dialog.setIcon(kdk::KMessageBox::Question);
    dialog.setWindowTitle(title);
    dialog.setText(text);
    QPushButton *acceptButton = dialog.addButton(acceptText, kdk::KMessageBox::AcceptRole);
    if (!cancelText.isEmpty()) {
        dialog.addButton(cancelText, kdk::KMessageBox::RejectRole);
    }
    dialog.exec();
    return dialog.clickedButton() == acceptButton;
}

QStringList apiServerBlock()
{
    return {
        QStringLiteral("  api_server:"),
        QStringLiteral("    extra:"),
        QStringLiteral("      port: 8642"),
        QStringLiteral("      host: 127.0.0.1"),
        QStringLiteral("    enabled: true"),
        QStringLiteral("    key: ''"),
        QStringLiteral("    cors_origins: '*'")
    };
}

bool isTopLevelLine(const QString &line)
{
    return !line.trimmed().isEmpty() &&
           !line.startsWith(QLatin1Char(' ')) &&
           !line.startsWith(QLatin1Char('\t'));
}

bool isPlatformsLine(const QString &line)
{
    return isTopLevelLine(line) && line.trimmed().startsWith(QStringLiteral("platforms:"));
}

bool isIndentedPlatformsChild(const QString &line)
{
    return line.startsWith(QStringLiteral("  ")) && !line.startsWith(QStringLiteral("    "));
}

bool isApiServerLine(const QString &line)
{
    return isIndentedPlatformsChild(line) && line.trimmed().startsWith(QStringLiteral("api_server:"));
}

bool ensureSectionInteger(QStringList &lines,
                          const QString &section,
                          const QString &key,
                          int value)
{
    const QString sectionLine = section + QLatin1Char(':');
    int sectionIndex = -1;
    int sectionEnd = lines.size();
    for (int i = 0; i < lines.size(); ++i) {
        if (!isTopLevelLine(lines.at(i)) || lines.at(i).trimmed() != sectionLine) {
            continue;
        }
        sectionIndex = i;
        for (int j = i + 1; j < lines.size(); ++j) {
            if (isTopLevelLine(lines.at(j))) {
                sectionEnd = j;
                break;
            }
        }
        break;
    }

    const QString desiredLine = QStringLiteral("  %1: %2").arg(key).arg(value);
    if (sectionIndex < 0) {
        if (!lines.isEmpty() && !lines.constLast().trimmed().isEmpty()) {
            lines.append(QString());
        }
        lines.append(sectionLine);
        lines.append(desiredLine);
        return true;
    }

    const QRegularExpression keyPattern(
        QStringLiteral("^  %1\\s*:\\s*[^#]*(\\s+#.*)?$")
            .arg(QRegularExpression::escape(key)));
    for (int i = sectionIndex + 1; i < sectionEnd; ++i) {
        const QRegularExpressionMatch match = keyPattern.match(lines.at(i));
        if (!match.hasMatch()) {
            continue;
        }
        const QString comment = match.captured(1);
        const QString replacement = desiredLine + comment;
        if (lines.at(i) == replacement) {
            return false;
        }
        lines[i] = replacement;
        return true;
    }

    lines.insert(sectionEnd, desiredLine);
    return true;
}

QStringList readLines(const QString &path)
{
    QFile file(path);
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

bool writeLines(const QString &path, const QStringList &lines)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }

    QTextStream out(&file);
    out << lines.join(QLatin1Char('\n'));
    if (!lines.isEmpty()) {
        out << '\n';
    }
    return file.commit();
}

GatewayService *g_activeGatewayService = nullptr;

} // namespace

GatewayService::GatewayService(QObject *parent)
    : QObject(parent)
    , m_gatewayProcess(nullptr)
    , m_progressDialog(nullptr)
    , m_progressStatus(nullptr)
    , m_progressLog(nullptr)
    , m_progressCancelButton(nullptr)
    , m_progressSessionActive(false)
    , m_progressCancelled(false)
    , m_progressRetryRequested(false)
    , m_startupRuntimeUserCancelled(false)
{
    g_activeGatewayService = this;
}

GatewayService::~GatewayService()
{
    if (g_activeGatewayService == this) {
        g_activeGatewayService = nullptr;
    }
    stopManagedGateway();
}

GatewayService *GatewayService::activeInstance()
{
    return g_activeGatewayService;
}

bool GatewayService::ensureReady()
{
    const bool forceRuntimeRestart =
        qEnvironmentVariable("KYLIN_AGENT_FORCE_RUNTIME_RESTART") == QStringLiteral("1");
    const QStringList configBefore = readLines(configPath());
    bool limitsConfigOk = ensureRuntimeLimitsConfig();
    bool apiConfigOk = ensureApiServerConfig();
    if (!limitsConfigOk || !apiConfigOk) {
        qWarning() << "Refusing to start kylin-agent-runtime service because config.yaml could not be updated";
        return false;
    }

    // A running gateway has already loaded its configuration. Restart it only
    // when startup normalization changed config.yaml; otherwise keep reusing it.
    const bool configChanged = configBefore != readLines(configPath());
    if (configChanged && isGatewayRunning() && !stopGateway()) {
        qWarning() << "Failed to stop kylin-agent-runtime service after updating config.yaml";
        return false;
    }

    bool retryingRuntimeSetup = false;
    while (true) {
        const bool gatewayWasRunning = isGatewayRunning();
        m_startupRuntimeUserCancelled = false;
        if (ensureStartupRuntimes(gatewayWasRunning, retryingRuntimeSetup)) {
            break;
        }
        retryingRuntimeSetup = false;
        if (m_progressRetryRequested) {
            m_progressRetryRequested = false;
            m_progressCancelled = false;
            m_startupRuntimeUserCancelled = false;
            retryingRuntimeSetup = true;
            continue;
        }
        if (m_startupRuntimeUserCancelled) {
            return false;
        }
        if (!confirmRetryStartupRuntime()) {
            return false;
        }
    }

    // The installer normally preserves config.yaml, but validate it again
    // after runtime preparation so the service never starts with stale values.
    limitsConfigOk = ensureRuntimeLimitsConfig();
    if (!limitsConfigOk) {
        qWarning() << "Failed to ensure kylin-agent-runtime turn and iteration limits";
    }
    apiConfigOk = ensureApiServerConfig();
    if (!apiConfigOk) {
        qWarning() << "Failed to ensure kylin-agent-runtime service API server config";
    }
    const bool configOk = apiConfigOk && limitsConfigOk;
    if (!configOk) {
        qWarning() << "Refusing to start kylin-agent-runtime service with an invalid config.yaml";
        return false;
    }

    // Development builds may reuse an already-running user service whose
    // process has not loaded locally updated runtime code yet. Stop it after
    // runtime preparation so the normal startup path below launches it once
    // with the current on-disk code.
    if (forceRuntimeRestart && isGatewayRunning()) {
        qDebug() << "Forcing kylin-agent-runtime service restart because "
                    "KYLIN_AGENT_FORCE_RUNTIME_RESTART=1";
        if (!stopGateway()) {
            qWarning() << "Failed to stop kylin-agent-runtime service for forced restart";
            return false;
        }
    }

    const bool servicesInstalled = ensureGatewayServicesInstalled();
    if (!servicesInstalled) {
        qWarning() << "Unable to install the kylin-agent-runtime user services; falling back to a process owned by KylinAgent";
    } else if (!isGatewayServiceActive() && isGatewayRunning()) {
        // Migrate a gateway left over from the old GUI-owned process or from
        // hermes-gateway.service before starting the authoritative unit.
        if (!stopGateway()) {
            qWarning() << "Failed to stop the legacy gateway process before service migration";
            return false;
        }
    }

    if (servicesInstalled && !isGatewayServiceActive()) {
        return startGateway();
    }

    if (isApiServerAvailable()) {
        qDebug() << "kylin-agent-runtime service API server is available";
        return configOk;
    }

    if (isGatewayRunning()) {
        qWarning() << "kylin-agent-runtime service is running but its API is not reachable on 127.0.0.1:8642";
        return configOk;
    }

    return startGateway();
}

bool GatewayService::restartGateway()
{
    const bool wasRunning = isGatewayRunning();
    if (wasRunning && !stopGateway()) {
        qWarning() << "Failed to stop kylin-agent-runtime service before restart";
        return false;
    }
    return startGateway();
}

void GatewayService::stopManagedGateway()
{
    if (!m_gatewayProcess) {
        return;
    }

    if (m_gatewayProcess->state() != QProcess::NotRunning) {
        qDebug() << "Stopping kylin-agent-runtime service process started by KylinAgent";
        m_gatewayProcess->terminate();
        if (!m_gatewayProcess->waitForFinished(kGatewayStopTimeoutMs)) {
            m_gatewayProcess->kill();
            m_gatewayProcess->waitForFinished(1000);
        }
    }
    m_gatewayProcess->deleteLater();
    m_gatewayProcess = nullptr;
}

QString GatewayService::runtimeExecutable() const
{
    QString executable = QStandardPaths::findExecutable(QStringLiteral("kylin-agent-runtime"));
    if (!executable.isEmpty()) {
        return executable;
    }

    const QStringList fallbacks = {
        QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).filePath(QStringLiteral(".local/bin/kylin-agent-runtime")),
        QStringLiteral("/usr/local/bin/kylin-agent-runtime"),
        QStringLiteral("/usr/bin/kylin-agent-runtime")
    };
    for (const QString &candidate : fallbacks) {
        const QFileInfo info(candidate);
        if (info.exists() && info.isExecutable()) {
            return candidate;
        }
    }

    return {};
}

QString GatewayService::runtimeHome() const
{
    const QByteArray agentRuntimeHome = qgetenv("AGENT_RUNTIME_HOME");
    if (!agentRuntimeHome.isEmpty()) {
        return QString::fromLocal8Bit(agentRuntimeHome);
    }

    // Keep the old environment variable as a compatibility fallback for
    // existing installations. New integrations should use AGENT_RUNTIME_HOME.
    const QByteArray envHome = qgetenv("HERMES_HOME");
    if (!envHome.isEmpty()) {
        return QString::fromLocal8Bit(envHome);
    }
    return QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation))
        .filePath(QStringLiteral(".kylin-agent-runtime"));
}

QString GatewayService::configPath() const
{
    return QDir(runtimeHome()).filePath(QStringLiteral("config.yaml"));
}

QString GatewayService::runtimeRoot() const
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation))
        .filePath(QStringLiteral(".local/share/kylin-agent"));
}

QString GatewayService::cuaRuntimePath() const
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation))
        .filePath(QStringLiteral(".local/share/kylin-cua"));
}

QString GatewayService::cuaRuntimeSourcePath() const
{
    return QDir(runtimeCacheSourcePath()).filePath(QStringLiteral("local/share/kylin-cua"));
}

QString GatewayService::cuaSkillSourcePath() const
{
    return QDir(cuaRuntimePath()).filePath(QStringLiteral("skill/kylin-cua"));
}

QString GatewayService::cuaSkillInstallPath() const
{
    return QDir(runtimeHome()).filePath(QStringLiteral("skills/kylin-cua"));
}

QString GatewayService::runtimeSkillsInstallPath() const
{
    return QDir(runtimeRoot()).filePath(QStringLiteral("skills"));
}

QString GatewayService::runtimeSkillsSourcePath() const
{
    return QStringLiteral("/usr/local/share/kylin-agent-runtime/skills");
}

QString GatewayService::runtimeInstallPath() const
{
    return QDir(runtimeHome()).filePath(QStringLiteral("agent-runtime"));
}

QString GatewayService::runtimeCacheSourcePath() const
{
    return QStringLiteral("/usr/local/share/kylin-agent-runtime");
}

QString GatewayService::runtimeCacheMarkerPath() const
{
    return QDir(runtimeHome()).filePath(QStringLiteral("cache-installed"));
}

QString GatewayService::gatewayServicePath() const
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation))
        .filePath(QStringLiteral(".config/systemd/user/") + kGatewayServiceName);
}

QString GatewayService::gatewayServiceDefinition() const
{
    const QString executable = QFileInfo(runtimeExecutable()).absoluteFilePath();
    const QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    const QString path = QProcessEnvironment::systemEnvironment().value(QStringLiteral("PATH"));
    return QStringLiteral(
        "[Unit]\n"
        "Description=Kylin Agent Runtime Gateway\n"
        "After=network-online.target\n"
        "Wants=network-online.target\n"
        "StartLimitIntervalSec=60\n"
        "StartLimitBurst=4\n\n"
        "[Service]\n"
        "Type=simple\n"
        "ExecStart=%1 gateway run --replace\n"
        "WorkingDirectory=%h\n"
        "Environment=%2\n"
        "Environment=%3\n"
        "Environment=%4\n"
        "Environment=%5\n"
        "Restart=on-failure\n"
        "RestartSec=5\n"
        "RestartForceExitStatus=75\n"
        "KillMode=mixed\n"
        "KillSignal=SIGTERM\n"
        "ExecReload=/bin/kill -USR1 $MAINPID\n"
        "TimeoutStopSec=90\n"
        "StandardOutput=journal\n"
        "StandardError=journal\n\n"
        "[Install]\n"
        "WantedBy=default.target\n")
        .arg(systemdQuote(executable),
             systemdQuote(QStringLiteral("HOME=") + home),
             systemdQuote(QStringLiteral("HERMES_HOME=") + runtimeHome()),
             systemdQuote(QStringLiteral("AGENT_RUNTIME_HOME=") + runtimeHome()),
             systemdQuote(QStringLiteral("PATH=") + path));
}

bool GatewayService::hasRuntimeExecutable() const
{
    return !runtimeExecutable().isEmpty();
}

bool GatewayService::hasBackendRuntime() const
{
    return QFileInfo(runtimeInstallPath()).isDir();
}

bool GatewayService::hasRuntimeVenv() const
{
    return QFileInfo(QDir(runtimeInstallPath()).filePath(
        QStringLiteral("venv/bin/kylin-agent-runtime"))).isExecutable();
}

bool GatewayService::hasCompleteRuntime() const
{
    return hasRuntimeExecutable() && hasBackendRuntime() && hasRuntimeVenv();
}

bool GatewayService::hasRuntimeCachePackage() const
{
    const QFileInfo sourceInfo(runtimeCacheSourcePath());
    if (!sourceInfo.exists() || !sourceInfo.isDir()) {
        return false;
    }

    const QDir sourceDir(runtimeCacheSourcePath());
    return QFileInfo(sourceDir.filePath(QStringLiteral("local"))).isDir() ||
           QFileInfo(sourceDir.filePath(QStringLiteral("cache"))).isDir();
}

bool GatewayService::hasRuntimeCacheInstalled() const
{
    return QFileInfo(runtimeCacheMarkerPath()).isFile();
}

QString GatewayService::versionFilePath(const QString &directory) const
{
    return QDir(directory).filePath(QStringLiteral("version"));
}

bool GatewayService::ensureRuntimeLimitsConfig()
{
    const QString path = configPath();
    QStringList lines = readLines(path);
    bool changed = false;
    changed |= ensureSectionInteger(lines, QStringLiteral("agent"),
                                    QStringLiteral("max_turns"), 600);
    changed |= ensureSectionInteger(lines, QStringLiteral("delegation"),
                                    QStringLiteral("max_iterations"), 500);
    changed |= ensureSectionInteger(lines, QStringLiteral("goals"),
                                    QStringLiteral("max_turns"), 200);
    return !changed || writeLines(path, lines);
}

bool GatewayService::ensureApiServerConfig()
{
    const QString path = configPath();
    QStringList lines = readLines(path);
    const QStringList desiredBlock = apiServerBlock();

    int platformsIndex = -1;
    int platformsEnd = lines.size();
    for (int i = 0; i < lines.size(); ++i) {
        if (!isPlatformsLine(lines.at(i))) {
            continue;
        }
        platformsIndex = i;
        if (lines.at(i).trimmed() != QStringLiteral("platforms:")) {
            lines[i] = QStringLiteral("platforms:");
        }
        for (int j = i + 1; j < lines.size(); ++j) {
            if (isTopLevelLine(lines.at(j))) {
                platformsEnd = j;
                break;
            }
        }
        break;
    }

    if (platformsIndex < 0) {
        if (!lines.isEmpty() && !lines.last().trimmed().isEmpty()) {
            lines.append(QString());
        }
        lines.append(QStringLiteral("platforms:"));
        lines.append(desiredBlock);
        return writeLines(path, lines);
    }

    int apiServerIndex = -1;
    int apiServerEnd = platformsEnd;
    for (int i = platformsIndex + 1; i < platformsEnd; ++i) {
        if (!isApiServerLine(lines.at(i))) {
            continue;
        }
        apiServerIndex = i;
        for (int j = i + 1; j < platformsEnd; ++j) {
            if (isIndentedPlatformsChild(lines.at(j))) {
                apiServerEnd = j;
                break;
            }
        }
        break;
    }

    if (apiServerIndex < 0) {
        for (int i = desiredBlock.size() - 1; i >= 0; --i) {
            lines.insert(platformsIndex + 1, desiredBlock.at(i));
        }
        return writeLines(path, lines);
    }

    const QStringList existingBlock = lines.mid(apiServerIndex, apiServerEnd - apiServerIndex);
    if (existingBlock == desiredBlock) {
        return true;
    }

    for (int i = apiServerEnd - 1; i >= apiServerIndex; --i) {
        lines.removeAt(i);
    }
    for (int i = desiredBlock.size() - 1; i >= 0; --i) {
        lines.insert(apiServerIndex, desiredBlock.at(i));
    }
    return writeLines(path, lines);
}

bool GatewayService::ensureRuntimeSkillsSeeded()
{
    const QString targetPath = runtimeSkillsInstallPath();
    const QFileInfo targetInfo(targetPath);
    if (targetInfo.exists()) {
        if (!targetInfo.isDir()) {
            qWarning() << "Runtime skills target exists but is not a directory:" << targetPath;
            return false;
        }
        return true;
    }

    const QString sourcePath = runtimeSkillsSourcePath();
    const QFileInfo sourceInfo(sourcePath);
    if (!sourceInfo.exists()) {
        qDebug() << "Runtime skills source not found; skipping seed:" << sourcePath;
        return true;
    }
    if (!sourceInfo.isDir()) {
        qWarning() << "Runtime skills source exists but is not a directory:" << sourcePath;
        return false;
    }

    QDir().mkpath(QFileInfo(targetPath).absolutePath());

    if (!confirmAction(
            tr("需要授权"),
            tr("初始化智能体技能需要管理员授权。是否继续？"),
            tr("继续"))) {
        qDebug() << "User cancelled runtime skills authorization";
        m_startupRuntimeUserCancelled = true;
        return false;
    }

    const QStringList seederCandidates = {
        QStringLiteral("/usr/local/share/kylin-agent/scripts/seed_runtime_skills.sh"),
        QStringLiteral("/usr/share/kylin-agent/scripts/seed_runtime_skills.sh"),
        QDir(QStringLiteral(KYLIN_AGENT_SOURCE_DIR)).filePath(QStringLiteral("scripts/seed_runtime_skills.sh"))
    };

    QString seederPath;
    for (const QString &candidate : seederCandidates) {
        const QFileInfo info(candidate);
        if (info.exists() && info.isFile()) {
            seederPath = candidate;
            break;
        }
    }

    if (seederPath.isEmpty()) {
        qWarning() << "Runtime skills seeder script not found";
        return false;
    }

    const QString invocation = QStringLiteral("%1 %2 %3")
        .arg(shellQuote(seederPath), shellQuote(sourcePath), shellQuote(targetPath));
    const QString command = QStringLiteral(
        "if command -v pkexec >/dev/null 2>&1; then "
        "pkexec %1; "
        "else "
        "sudo %1; "
        "fi")
        .arg(invocation);

    QString output;
    const bool ok = runShellCommand(
        command,
        kNoInstallTimeoutMs,
        &output,
        tr("初始化智能体技能"),
        tr("复制运行时技能"));
    if (!ok) {
        qWarning().noquote() << "Failed to seed runtime skills:" << output;
        return false;
    }

    qDebug() << "Seeded runtime skills from" << sourcePath << "to" << targetPath;
    return true;
}

bool GatewayService::ensureStartupRuntimes(bool gatewayWasRunning, bool skipConfirm)
{
    const bool runtimeCacheAvailable = hasRuntimeCachePackage();
    const bool needRuntimeCacheInstall = runtimeCacheAvailable && !hasRuntimeCacheInstalled();
    bool needCuaUpdate = needsCuaRuntimeUpdate();
    bool needCuaSkillSync = needsCuaSkillSync();
    bool needRuntimeInstall = !hasRuntimeExecutable();
    bool needRuntimeVenvInstall = !hasRuntimeVenv();
    bool needBackendUpdate = needsBackendRuntimeUpdate();
    bool needBackendRuntimeInstall = !hasBackendRuntime();
    bool needsRuntimeAction = needRuntimeCacheInstall || needCuaUpdate || needCuaSkillSync ||
                              needRuntimeInstall || needRuntimeVenvInstall ||
                              needBackendRuntimeInstall || needBackendUpdate;

    qDebug() << "Runtime startup check"
             << "runtimeCacheAvailable:" << runtimeCacheAvailable
             << "needRuntimeCacheInstall:" << needRuntimeCacheInstall
             << "needCuaUpdate:" << needCuaUpdate
             << "needCuaSkillSync:" << needCuaSkillSync
             << "needRuntimeInstall:" << needRuntimeInstall
             << "needRuntimeVenvInstall:" << needRuntimeVenvInstall
             << "needBackendRuntimeInstall:" << needBackendRuntimeInstall
             << "needBackendUpdate:" << needBackendUpdate
             << "runtimeExecutable:" << runtimeExecutable()
             << "hasRuntimeVenv:" << hasRuntimeVenv()
             << "cuaVersion:" << readTrimmedFile(versionFilePath(cuaRuntimePath()))
             << "expectedCuaVersion:" << QStringLiteral(KYLIN_AGENT_CUA_VERSION)
             << "cuaSourceVersion:" << readTrimmedFile(versionFilePath(cuaRuntimeSourcePath()))
             << "cuaHasGit:" << QFileInfo(QDir(cuaRuntimePath()).filePath(QStringLiteral(".git"))).isDir()
             << "cuaSkillSourceVersion:" << readTrimmedFile(versionFilePath(cuaSkillSourcePath()))
             << "cuaSkillInstalledVersion:" << readTrimmedFile(versionFilePath(cuaSkillInstallPath()))
             << "expectedBackendVersion:" << QStringLiteral(KYLIN_AGENT_RUNTIME_VERSION)
             << "runtimeInstallVersion:" << readTrimmedFile(versionFilePath(runtimeInstallPath()));

    if (!needsRuntimeAction) {
        if (gatewayWasRunning) {
            qDebug() << "kylin-agent-runtime service and managed runtimes are current; reusing existing service";
        }
        return true;
    }

    if (needsRuntimeAction && !skipConfirm) {
        QStringList items;
        if (needRuntimeCacheInstall) {
            items.append(tr("智能体离线运行时缓存"));
        }
        if (needRuntimeInstall || needRuntimeVenvInstall || needCuaUpdate || needCuaSkillSync ||
            needBackendRuntimeInstall || needBackendUpdate) {
            items.append(tr("智能体运行时"));
        }

        if (!confirmAction(
                tr("更新智能体运行时"),
                tr("检测到以下组件需要安装或更新：\n\n%1\n\n是否现在执行？")
                    .arg(QStringLiteral("- ") + items.join(QStringLiteral("\n- "))),
                tr("执行"))) {
            qDebug() << "User cancelled startup runtime installation/update";
            m_startupRuntimeUserCancelled = true;
            return false;
        }
    }

    // A live process can keep serving from deleted or outdated files, so its
    // active state must never bypass the on-disk checks above. Stop it only
    // after the user accepts the update, then let ensureReady() restore the
    // authoritative user service after installation completes.
    if (gatewayWasRunning) {
        qDebug() << "Stopping kylin-agent-runtime service before managed runtime update";
        if (!stopGateway()) {
            qWarning() << "Failed to stop kylin-agent-runtime service before runtime update";
            return false;
        }
        gatewayWasRunning = false;
    }

    if (needsRuntimeAction) {
        beginProgressSession(tr("更新智能体运行时"), tr("准备安装/更新智能体运行时"));
    }

    bool ok = true;
    bool runtimeCacheInstalledNow = false;
    if (needRuntimeCacheInstall) {
        if (!runRuntimeCacheInstaller()) {
            ok = false;
        } else {
            runtimeCacheInstalledNow = true;
            needCuaUpdate = needsCuaRuntimeUpdate();
            needCuaSkillSync = needsCuaSkillSync();
            needRuntimeInstall = !hasRuntimeExecutable();
            needRuntimeVenvInstall = !hasRuntimeVenv();
            needBackendUpdate = needsBackendRuntimeUpdate();
            needBackendRuntimeInstall = !hasBackendRuntime();
            needsRuntimeAction = needRuntimeCacheInstall || needCuaUpdate || needCuaSkillSync ||
                                  needRuntimeInstall || needRuntimeVenvInstall ||
                                  needBackendRuntimeInstall || needBackendUpdate;
        }
    }

    if (ok && (needCuaUpdate || needCuaSkillSync) && !ensureCuaRuntime()) {
        qWarning() << "Kylin CUA runtime setup failed; continuing with kylin-agent-runtime service setup";
    }

    if (ok && runtimeCacheInstalledNow &&
        (needRuntimeInstall || needRuntimeVenvInstall || needBackendRuntimeInstall || needBackendUpdate)) {
        if (!runRuntimeInstaller()) {
            qWarning() << "Failed to complete runtime setup after installing packaged runtime cache";
            ok = false;
        }
    }

    if (ok && !runtimeCacheInstalledNow && (needRuntimeInstall || needRuntimeVenvInstall) && !ensureRuntimeInstalled()) {
        ok = false;
    }

    if (ok && hasRuntimeExecutable()) {
        if (!ensureBackendRuntime(gatewayWasRunning, false)) {
            ok = false;
        }
    } else if (ok) {
        qWarning() << "kylin-agent-runtime service executable is still unavailable";
        ok = false;
    }

    if (ok && !hasCompleteRuntime()) {
        qWarning() << "kylin-agent-runtime service is incomplete"
                   << "hasExecutable:" << hasRuntimeExecutable()
                   << "hasBackendRuntime:" << hasBackendRuntime()
                   << "hasRuntimeVenv:" << hasRuntimeVenv()
                   << "backendPath:" << runtimeInstallPath();
        ok = false;
    }

    if (needsRuntimeAction) {
        endProgressSession(ok);
    }

    return ok;
}

bool GatewayService::ensureRuntimeInstalled()
{
    qDebug() << "kylin-agent-runtime executable not found; installing Kylin Agent runtime with bundled installer";

    if (!runRuntimeInstaller()) {
        return false;
    }

    if (runtimeExecutable().isEmpty()) {
        qWarning() << "Kylin Agent runtime installer completed, but kylin-agent-runtime was not found";
        return false;
    }

    qDebug() << "kylin-agent-runtime installed at" << runtimeExecutable();
    return true;
}

bool GatewayService::runRuntimeInstaller()
{
    const QStringList installerCandidates = {
        QStringLiteral("/usr/local/share/kylin-agent/scripts/agent_runtime_install.sh"),
        QStringLiteral("/usr/share/kylin-agent/scripts/agent_runtime_install.sh"),
        QDir(QStringLiteral(KYLIN_AGENT_SOURCE_DIR)).filePath(QStringLiteral("scripts/agent_runtime_install.sh"))
    };

    QString installerPath;
    for (const QString &candidate : installerCandidates) {
        const QFileInfo info(candidate);
        if (info.exists() && info.isFile()) {
            installerPath = candidate;
            break;
        }
    }

    if (installerPath.isEmpty()) {
        qWarning() << "Kylin Agent runtime installer script not found";
        return false;
    }

    const QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString cacheEnvironment = QStringLiteral("HOME=%1 UV_CACHE_DIR=%2 PIP_CACHE_DIR=%3 ")
                                   .arg(shellQuote(homePath),
                                        shellQuote(QDir(homePath).filePath(QStringLiteral(".cache/uv"))),
                                        shellQuote(QDir(homePath).filePath(QStringLiteral(".cache/pip"))));
    if (hasRuntimeCacheInstalled()) {
        cacheEnvironment += QStringLiteral("UV_OFFLINE=1 ");
    }

    QString output;
    const bool ok = runShellCommand(
        QStringLiteral("%1AGENT_RUNTIME_HOME=%2 HERMES_HOME=%2 INSTALL_HERMES_LAUNCHER=0 KYLIN_AGENT_EXPECTED_RUNTIME_VERSION=%3 bash %4 --non-interactive --skip-setup")
            .arg(cacheEnvironment,
                 shellQuote(runtimeHome()),
                 shellQuote(QStringLiteral(KYLIN_AGENT_RUNTIME_VERSION)),
                 shellQuote(installerPath)),
        kNoInstallTimeoutMs,
        &output,
        tr("安装智能体运行时"),
        tr("安装/更新智能体运行时"),
        !m_progressSessionActive);
    if (!ok) {
        qWarning().noquote() << "Kylin Agent runtime installer failed:" << output;
        return false;
    }

    if (!updateDefaultSoulMd()) {
        qWarning() << "Failed to update default SOUL.md after runtime installer completed";
    }

    return true;
}

bool GatewayService::runRuntimeCacheInstaller()
{
    const QStringList installerCandidates = {
        QDir(runtimeCacheSourcePath()).filePath(QStringLiteral("install_runtime_cache.sh")),
        QStringLiteral("/usr/local/share/kylin-agent/scripts/install_runtime_cache.sh"),
        QStringLiteral("/usr/share/kylin-agent/scripts/install_runtime_cache.sh"),
        QDir(QStringLiteral(KYLIN_AGENT_SOURCE_DIR)).filePath(QStringLiteral("scripts/install_runtime_cache.sh"))
    };

    QString installerPath;
    for (const QString &candidate : installerCandidates) {
        const QFileInfo info(candidate);
        if (info.exists() && info.isFile()) {
            installerPath = candidate;
            break;
        }
    }

    if (installerPath.isEmpty()) {
        qWarning() << "Kylin Agent runtime cache installer script not found";
        return false;
    }

    const QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString output;
    const bool ok = runShellCommand(
        QStringLiteral("HOME=%1 AGENT_RUNTIME_HOME=%2 HERMES_HOME=%2 RUNTIME_SHARE_DIR=%3 bash %4 %1")
            .arg(shellQuote(homePath),
                 shellQuote(runtimeHome()),
                 shellQuote(runtimeCacheSourcePath()),
                 shellQuote(installerPath)),
        kNoInstallTimeoutMs,
        &output,
        tr("安装智能体运行时缓存"),
        tr("复制离线运行时缓存"),
        !m_progressSessionActive);
    if (!ok) {
        qWarning().noquote() << "Kylin Agent runtime cache installer failed:" << output;
        return false;
    }

    if (!hasRuntimeCacheInstalled()) {
        qWarning() << "Runtime cache installer completed, but marker was not found:" << runtimeCacheMarkerPath();
        return false;
    }

    qDebug() << "Runtime cache installed from" << runtimeCacheSourcePath();
    return true;
}

bool GatewayService::updateDefaultSoulMd() const
{
    QDir().mkpath(runtimeHome());

    const QString path = QDir(runtimeHome()).filePath(QStringLiteral("SOUL.md"));
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }

    QTextStream out(&file);
    out << defaultSoulMd();
    return file.commit();
}

bool GatewayService::ensureCuaRuntime()
{
    if (needsCuaRuntimeUpdate()) {
        qDebug() << "Kylin CUA runtime update required"
                 << "version:" << readTrimmedFile(versionFilePath(cuaRuntimePath()))
                 << "expectedVersion:" << QStringLiteral(KYLIN_AGENT_CUA_VERSION)
                 << "sourceVersion:" << readTrimmedFile(versionFilePath(cuaRuntimeSourcePath()))
                 << "hasGitDir:" << QFileInfo(QDir(cuaRuntimePath()).filePath(QStringLiteral(".git"))).isDir();

        if (!updateCuaRuntime()) {
            showMessageDialog(QApplication::activeWindow(),
                              tr("CUA 更新失败"),
                              tr("无法下载或更新 Kylin CUA 运行时，将继续安装/更新其他智能体运行时组件。"),
                              tr("确定"));
            return false;
        }
    }

    const QString installedVersion = readTrimmedFile(versionFilePath(cuaRuntimePath()));
    const QString expectedVersion = QStringLiteral(KYLIN_AGENT_CUA_VERSION);
    if (installedVersion.isEmpty() || isVersionOlderThan(installedVersion, expectedVersion)) {
        showMessageDialog(QApplication::activeWindow(),
                          tr("CUA 更新失败"),
                          tr("Kylin CUA 运行时版本校验失败（当前：%1，要求：%2），将继续安装/更新其他智能体运行时组件。")
                              .arg(installedVersion.isEmpty() ? tr("未知") : installedVersion,
                                   expectedVersion),
                          tr("确定"));
        return false;
    }

    if (needsCuaSkillSync() && !syncCuaSkill()) {
        showMessageDialog(QApplication::activeWindow(),
                          tr("CUA 更新失败"),
                          tr("无法同步 CUA 智能体技能，将继续安装/更新其他智能体运行时组件。"),
                          tr("确定"));
        return false;
    }

    return true;
}

bool GatewayService::hasYdotoolEnvironment() const
{
    QString executable = QStandardPaths::findExecutable(QStringLiteral("ydotool"));
    if (executable.isEmpty() && QFileInfo(QStringLiteral("/usr/local/bin/ydotool")).isExecutable()) {
        executable = QStringLiteral("/usr/local/bin/ydotool");
    }
    if (executable.isEmpty() && QFileInfo(QStringLiteral("/usr/bin/ydotool")).isExecutable()) {
        executable = QStringLiteral("/usr/bin/ydotool");
    }
    if (executable.isEmpty()) {
        qDebug() << "Wayland CUA probe: ydotool executable was not found";
        return false;
    }

    // A running systemd unit is not sufficient: ydotoold can be active while
    // failing to open /dev/uinput. Exercise the real client/backend path with
    // a zero-distance move so the readiness check has no visible cursor side
    // effect. A non-zero exit, crash, or timeout triggers the repair installer.
    QProcess process;
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    const QString runtimeDir = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    const QString socketPath = QDir(runtimeDir).filePath(QStringLiteral(".ydotool_socket"));
    if (QFileInfo::exists(socketPath)) {
        environment.insert(QStringLiteral("YDOTOOL_SOCKET"), socketPath);
    }
    process.setProcessEnvironment(environment);
    process.setProgram(executable);
    process.setArguments({QStringLiteral("mousemove"), QStringLiteral("0"), QStringLiteral("0")});
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start();
    if (!process.waitForFinished(kGatewayStatusTimeoutMs)) {
        process.kill();
        process.waitForFinished(1000);
        qWarning() << "Wayland CUA probe timed out:" << executable;
        return false;
    }

    const QString output = QString::fromUtf8(process.readAll()).trimmed();
    const bool ready = process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
    if (!ready) {
        qWarning().noquote() << "Wayland CUA probe failed; ydotool will be repaired:"
                             << output;
    }
    return ready;
}

bool GatewayService::ensureYdotoolEnvironment()
{
    if (hasYdotoolEnvironment()) {
        qDebug() << "Wayland CUA input environment is already ready";
        return true;
    }

    const QStringList installerCandidates = {
        QStringLiteral("/usr/local/share/kylin-agent/scripts/ydotool_install.sh"),
        QStringLiteral("/usr/share/kylin-agent/scripts/ydotool_install.sh"),
        QDir(QStringLiteral(KYLIN_AGENT_SOURCE_DIR)).filePath(QStringLiteral("scripts/ydotool_install.sh"))
    };
    QString installerPath;
    for (const QString &candidate : installerCandidates) {
        const QFileInfo info(candidate);
        if (info.exists() && info.isFile()) {
            installerPath = candidate;
            break;
        }
    }
    if (installerPath.isEmpty()) {
        qWarning() << "ydotool installer script not found";
        return false;
    }

    const QString userName = QProcessEnvironment::systemEnvironment()
                                 .value(QStringLiteral("USER"), qEnvironmentVariable("LOGNAME"));
    const QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    const QString downloadDir = QDir(home).filePath(QStringLiteral("Downloads/ydotool-src"));
    const QString invocation = QStringLiteral("bash %1 %2 %3")
        .arg(shellQuote(installerPath), shellQuote(downloadDir), shellQuote(userName));
    const QString command = QStringLiteral(
        "if command -v pkexec >/dev/null 2>&1; then "
        "pkexec %1; "
        "else "
        "sudo %1; "
        "fi").arg(invocation);

    QString output;
    const bool installed = runShellCommand(
        command,
        kNoInstallTimeoutMs,
        &output,
        tr("安装 CUA Wayland 环境"),
        tr("安装并启动 ydotoold 输入服务"));
    if (!installed) {
        qWarning().noquote() << "Failed to install ydotool environment:" << output;
        return false;
    }
    if (!hasYdotoolEnvironment()) {
        qWarning() << "ydotool installer completed, but ydotoold is not active";
        return false;
    }
    return true;
}

bool GatewayService::ensureBackendRuntime(bool gatewayWasRunning, bool promptBeforeUpdate)
{
    QString installedVersion = readTrimmedFile(versionFilePath(runtimeInstallPath()));
    const QString expectedVersion = QStringLiteral(KYLIN_AGENT_RUNTIME_VERSION);
    if (isVersionAtLeast(installedVersion, expectedVersion)) {
        return true;
    }

    if (installedVersion.isEmpty() || isVersionOlderThan(installedVersion, expectedVersion)) {
        qDebug() << "kylin-agent-runtime service version update required"
                 << "installed:" << installedVersion
                 << "expected:" << expectedVersion;

        if (promptBeforeUpdate) {
            if (!confirmAction(
                    tr("更新智能体运行时"),
                    tr("检测到智能体运行时缺失或版本不一致，需要更新后端运行时。是否现在更新？"),
                    tr("更新"))) {
                qDebug() << "User cancelled kylin-agent-runtime service update";
                return false;
            }
        }

        if (!runRuntimeInstaller()) {
            return false;
        }

        QString scriptInstalledVersion = readTrimmedFile(versionFilePath(runtimeInstallPath()));
        if (scriptInstalledVersion.isEmpty()) {
            qWarning() << "Runtime installer completed, but no backend runtime version file was found";
            return false;
        }
        if (isVersionOlderThan(scriptInstalledVersion, expectedVersion)) {
            qWarning() << "kylin-agent-runtime service version mismatch after installer"
                       << "installed:" << scriptInstalledVersion
                       << "expected:" << expectedVersion;
            return false;
        }
    }

    return true;
}

bool GatewayService::needsCuaRuntimeUpdate() const
{
    const QString localVersion = readTrimmedFile(versionFilePath(cuaRuntimePath()));
    return localVersion.isEmpty() ||
           isVersionOlderThan(localVersion, QStringLiteral(KYLIN_AGENT_CUA_VERSION));
}

bool GatewayService::needsCuaSkillSync() const
{
    const QString sourceVersion = readTrimmedFile(versionFilePath(cuaSkillSourcePath()));
    if (sourceVersion.isEmpty()) {
        return false;
    }

    const QString installedVersion = readTrimmedFile(versionFilePath(cuaSkillInstallPath()));
    return installedVersion.isEmpty() || isVersionOlderThan(installedVersion, sourceVersion);
}

bool GatewayService::needsBackendRuntimeUpdate() const
{
    QString installedVersion = readTrimmedFile(versionFilePath(runtimeInstallPath()));
    return installedVersion.isEmpty() ||
           isVersionOlderThan(installedVersion, QStringLiteral(KYLIN_AGENT_RUNTIME_VERSION));
}

bool GatewayService::updateCuaRuntime()
{
    QDir().mkpath(QFileInfo(cuaRuntimePath()).absolutePath());

    const QString gitRetry = QStringLiteral(
        "git_retry() { n=1; while [ $n -le 3 ]; do "
        "git -c http.version=HTTP/1.1 -c core.compression=0 \"$@\" && return 0; "
        "echo \"git command failed (attempt $n/3), retrying...\" >&2; "
        "sleep $((n * 2)); n=$((n + 1)); done; return 1; }; ");
    const QString updateCommand = QStringLiteral(
        "export GIT_TERMINAL_PROMPT=0; ") + gitRetry + QStringLiteral(
        "repo=\"$HOME/.local/share/kylin-cua\"; "
        "legacy_repo=\"$HOME/.local/share/kylin-agent/kylin-cua\"; "
        "url=\"https://clxf66:bjfcmlc1@gitee.com/clxf66/kylin-cua.git\"; "
        "if [ ! -e \"$repo\" ] && git -C \"$legacy_repo\" rev-parse --verify HEAD >/dev/null 2>&1; then "
        "  mv \"$legacy_repo\" \"$repo\" || exit 1; "
        "fi; "
        "fresh_clone() { "
        "  replacement=\"${repo}.reclone.$$\"; previous=\"${repo}.previous.$$\"; "
        "  rm -rf \"$replacement\" \"$previous\"; "
        "  git_retry clone --depth 1 --single-branch --branch master \"$url\" \"$replacement\" || return 1; "
        "  git -C \"$replacement\" rev-parse --verify HEAD >/dev/null 2>&1 || { rm -rf \"$replacement\"; return 1; }; "
        "  if [ -e \"$repo\" ]; then mv \"$repo\" \"$previous\" || return 1; fi; "
        "  if mv \"$replacement\" \"$repo\"; then rm -rf \"$previous\"; return 0; fi; "
        "  [ ! -e \"$repo\" ] && [ -e \"$previous\" ] && mv \"$previous\" \"$repo\"; "
        "  rm -rf \"$replacement\"; return 1; "
        "}; "
        "if git -C \"$repo\" rev-parse --verify HEAD >/dev/null 2>&1; then "
        "  if ! { "
        "    git -C \"$repo\" merge --abort >/dev/null 2>&1 || true; "
        "    git -C \"$repo\" rebase --abort >/dev/null 2>&1 || true; "
        "    git -C \"$repo\" cherry-pick --abort >/dev/null 2>&1 || true; "
        "    git -C \"$repo\" reset --hard HEAD && "
        "    git -C \"$repo\" clean -ffd && "
        "    git -C \"$repo\" remote set-url origin \"$url\" && "
        "    git_retry -C \"$repo\" fetch --prune --depth 1 origin master && "
        "    git -C \"$repo\" checkout -B master FETCH_HEAD && "
        "    git -C \"$repo\" reset --hard FETCH_HEAD && "
        "    git -C \"$repo\" clean -ffd; "
        "  }; then fresh_clone; fi; "
        "else "
        "  fresh_clone; "
        "fi");

    QString output;
    const bool ok = runShellCommand(
        updateCommand,
        kNoInstallTimeoutMs,
        &output,
        tr("更新智能体运行时"),
        tr("安装/更新CUA智能体"));
    if (!ok) {
        qWarning().noquote() << "Failed to update kylin-cua:" << output;
        return false;
    }

    const QString pipCommand = QStringLiteral(
        "python3 -m pip install -r \"$HOME/.local/share/kylin-cua/requirements.txt\" --break-system-packages");
    QString pipOutput;
    const bool pipOk = runShellCommand(
        pipCommand,
        kNoInstallTimeoutMs,
        &pipOutput,
        tr("更新智能体运行时"),
        tr("安装/更新CUA智能体依赖"));
    if (!pipOk) {
        qWarning().noquote() << "Failed to install kylin-cua Python dependencies:" << pipOutput;
        return false;
    }
    return true;
}

bool GatewayService::syncCuaSkill()
{
    QDir().mkpath(QFileInfo(cuaSkillInstallPath()).absolutePath());
    QDir(QDir(runtimeHome()).filePath(QStringLiteral("import-skill/kylin-cua"))).removeRecursively();

    const QString command = QStringLiteral(
        "rsync -av \"%1/\" \"%2/\"")
        .arg(cuaSkillSourcePath(), cuaSkillInstallPath());

    QString output;
    const bool ok = runShellCommand(
        command,
        kNoInstallTimeoutMs,
        &output,
        tr("更新智能体运行时"),
        tr("安装/更新CUA智能体"));
    if (!ok) {
        qWarning().noquote() << "Failed to sync kylin-cua skill:" << output;
        return false;
    }

    const QString sourceVersion = readTrimmedFile(versionFilePath(cuaSkillSourcePath()));
    const QString installedVersion = readTrimmedFile(versionFilePath(cuaSkillInstallPath()));
    if (sourceVersion.isEmpty() || installedVersion.isEmpty() || isVersionOlderThan(installedVersion, sourceVersion)) {
        qWarning() << "Kylin CUA skill version mismatch after sync"
                   << "installed:" << installedVersion
                   << "source:" << sourceVersion;
        return false;
    }

    return true;
}

bool GatewayService::isApiServerAvailable() const
{
    QTcpSocket socket;
    socket.connectToHost(QStringLiteral("127.0.0.1"), kApiServerPort);
    return socket.waitForConnected(500);
}

bool GatewayService::isGatewayRunning() const
{
    if (isGatewayServiceActive()) {
        return true;
    }

    QString output;
    if (!runRuntimeCommand({QStringLiteral("gateway"), QStringLiteral("status")}, &output)) {
        return false;
    }
    return output.toLower().contains(QStringLiteral("gateway is running"));
}

bool GatewayService::isGatewayServiceActive() const
{
    return runSystemctl({QStringLiteral("is-active"), QStringLiteral("--quiet"),
                         kGatewayServiceName});
}

bool GatewayService::runSystemctl(const QStringList &arguments, QString *output) const
{
    const QString executable = QStandardPaths::findExecutable(QStringLiteral("systemctl"));
    if (executable.isEmpty()) {
        return false;
    }

    QProcess process;
    process.setProgram(executable);
    process.setArguments(QStringList{QStringLiteral("--user")} + arguments);
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start();
    if (!process.waitForFinished(kSystemctlTimeoutMs)) {
        process.kill();
        process.waitForFinished(1000);
        return false;
    }
    const QString text = QString::fromUtf8(process.readAll()).trimmed();
    if (output) {
        *output = text;
    }
    return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
}

bool GatewayService::disableLegacyGatewayServices()
{
    const QString unitDirectory = QFileInfo(gatewayServicePath()).absolutePath();
    const QStringList legacyUnits = {
        QStringLiteral("hermes-gateway.service"),
        QStringLiteral("hermes.service")
    };
    for (const QString &unit : legacyUnits) {
        const QString path = QDir(unitDirectory).filePath(unit);
        const QString definition = readTextFile(path).toLower();
        if (definition.isEmpty() ||
            (!definition.contains(QStringLiteral("gateway")) &&
             !definition.contains(QStringLiteral("kylin_agent_runtime_cli")))) {
            continue;
        }
        runSystemctl({QStringLiteral("disable"), QStringLiteral("--now"), unit});
        if (!QFile::remove(path)) {
            qWarning() << "Failed to remove legacy gateway user service:" << path;
            return false;
        }
        qDebug() << "Removed legacy gateway user service:" << unit;
    }
    return true;
}

bool GatewayService::ensureGatewayServicesInstalled()
{
    if (QStandardPaths::findExecutable(QStringLiteral("systemctl")).isEmpty()) {
        return false;
    }
    if (!runSystemctl({QStringLiteral("show-environment")})) {
        qWarning() << "systemd user manager is unavailable";
        return false;
    }

    const QString gatewayDefinition = gatewayServiceDefinition();
    if (gatewayDefinition.isEmpty()) {
        return false;
    }

    bool definitionsChanged = false;
    if (readTextFile(gatewayServicePath()) != gatewayDefinition) {
        if (!writeTextFile(gatewayServicePath(), gatewayDefinition)) {
            qWarning() << "Failed to install gateway user service:" << gatewayServicePath();
            return false;
        }
        definitionsChanged = true;
    }
    const QString notifierPath = QDir(QFileInfo(gatewayServicePath()).absolutePath())
        .filePath(kNotifierServiceName);
    runSystemctl({QStringLiteral("disable"), QStringLiteral("--now"), kNotifierServiceName});
    if (QFileInfo::exists(notifierPath)) {
        if (!QFile::remove(notifierPath)) {
            qWarning() << "Failed to remove obsolete desktop notifier service:" << notifierPath;
            return false;
        }
        definitionsChanged = true;
    }

    if (!disableLegacyGatewayServices()) {
        return false;
    }
    if (definitionsChanged && !runSystemctl({QStringLiteral("daemon-reload")})) {
        qWarning() << "Failed to reload systemd user services";
        return false;
    }
    if (!runSystemctl({QStringLiteral("enable"), kGatewayServiceName})) {
        qWarning() << "Failed to enable kylin-agent-runtime gateway service";
        return false;
    }

    if (isGatewayServiceActive()) {
        const QString action = definitionsChanged ? QStringLiteral("restart")
                                                  : QStringLiteral("start");
        QString output;
        if (!runSystemctl({action, kGatewayServiceName}, &output)) {
            qWarning().noquote() << "Failed to apply kylin-agent-runtime gateway service:" << output;
            return false;
        }
    }

    qDebug().noquote() << "kylin-agent-runtime gateway service ExecStart:"
                       << QFileInfo(runtimeExecutable()).absoluteFilePath()
                       << "gateway run --replace";
    return true;
}

bool GatewayService::stopGateway()
{
    qDebug() << "Stopping running kylin-agent-runtime service before backend update";

    stopManagedGateway();

    if (isGatewayServiceActive()) {
        QString serviceOutput;
        if (!runSystemctl({QStringLiteral("stop"), kGatewayServiceName}, &serviceOutput)) {
            qWarning().noquote() << "Failed to stop kylin-agent-runtime gateway service:"
                                 << serviceOutput;
            return false;
        }
    }

    QString output;
    if (runRuntimeCommand({QStringLiteral("gateway"), QStringLiteral("stop")}, &output)) {
        for (int i = 0; i < 10; ++i) {
            if (!isGatewayRunning()) {
                return true;
            }
            QThread::msleep(500);
        }
    } else if (!output.isEmpty()) {
        qWarning().noquote() << "kylin-agent-runtime service stop command failed:" << output;
    }

    const bool killed = runShellCommand(
        QStringLiteral("pids=$(pgrep -f '[h]ermes.*gateway.*run' || true); "
                       "if [ -n \"$pids\" ]; then kill $pids || true; sleep 1; "
                       "pids=$(pgrep -f '[h]ermes.*gateway.*run' || true); "
                       "if [ -n \"$pids\" ]; then kill -9 $pids || true; fi; fi"),
        kGatewayStopTimeoutMs,
        &output);
    if (!killed) {
        qWarning().noquote() << "Failed to kill kylin-agent-runtime service process:" << output;
        return false;
    }

    for (int i = 0; i < 10; ++i) {
        if (!isGatewayRunning()) {
            return true;
        }
        QThread::msleep(500);
    }
    return !isGatewayRunning();
}

bool GatewayService::startGateway()
{
    const QString executable = runtimeExecutable();
    if (executable.isEmpty()) {
        qWarning() << "Cannot start kylin-agent-runtime gateway: executable not found";
        return false;
    }

    if (QFileInfo::exists(gatewayServicePath())) {
        QString output;
        if (!runSystemctl({QStringLiteral("start"), kGatewayServiceName}, &output)) {
            qWarning().noquote() << "Failed to start kylin-agent-runtime gateway service:" << output;
            return false;
        }
        qDebug() << "Started" << kGatewayServiceName << "with" << executable;
        for (int i = 0; i < 20; ++i) {
            if (isApiServerAvailable()) {
                return true;
            }
            QThread::msleep(500);
        }
        qWarning() << "kylin-agent-runtime service started, but its API did not become reachable within 10 seconds";
        return true;
    }

    if (m_gatewayProcess) {
        stopManagedGateway();
    }

    m_gatewayProcess = new QProcess(this);
    m_gatewayProcess->setProgram(executable);
    m_gatewayProcess->setArguments({QStringLiteral("gateway"), QStringLiteral("run")});
    m_gatewayProcess->setProcessChannelMode(QProcess::MergedChannels);
    m_gatewayProcess->setWorkingDirectory(QDir::homePath());
    connect(m_gatewayProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        const QString output = QString::fromUtf8(m_gatewayProcess->readAllStandardOutput()).trimmed();
        if (!output.isEmpty()) {
            qDebug().noquote() << "kylin-agent-runtime service:" << output;
        }
    });
    connect(m_gatewayProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        qDebug() << "kylin-agent-runtime service process finished" << exitCode << exitStatus;
    });

    m_gatewayProcess->start();
    if (!m_gatewayProcess->waitForStarted(5000)) {
        qWarning() << "Failed to start kylin-agent-runtime service:" << m_gatewayProcess->errorString();
        m_gatewayProcess->deleteLater();
        m_gatewayProcess = nullptr;
        return false;
    }

    qDebug() << "Started kylin-agent-runtime service process from KylinAgent";
    for (int i = 0; i < 20; ++i) {
        if (isApiServerAvailable()) {
            return true;
        }
        QThread::msleep(500);
    }
    qWarning() << "kylin-agent-runtime service started, but its API did not become reachable within 10 seconds";
    return true;
}

bool GatewayService::runRuntimeCommand(const QStringList &arguments, QString *output) const
{
    const QString executable = runtimeExecutable();
    if (executable.isEmpty()) {
        return false;
    }

    QProcess process;
    process.setProgram(executable);
    process.setArguments(arguments);
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start();
    if (!process.waitForFinished(kGatewayStatusTimeoutMs)) {
        process.kill();
        process.waitForFinished(1000);
        return false;
    }

    const QString text = QString::fromUtf8(process.readAll()).trimmed();
    if (output) {
        *output = text;
    }
    return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
}

bool GatewayService::runShellCommand(const QString &command,
                                           int timeoutMs,
                                           QString *output,
                                           const QString &progressTitle,
                                           const QString &progressText,
                                           bool shouldSaveRuntimeUpdateLog) const
{
    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    const QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    const QString path = env.value(QStringLiteral("PATH"));
    const QString localBin = QDir(home).filePath(QStringLiteral(".local/bin"));
    if (!path.split(QLatin1Char(':')).contains(localBin)) {
        env.insert(QStringLiteral("PATH"), localBin + QLatin1Char(':') + path);
    }

    process.setProcessEnvironment(env);
    process.setProgram(QStringLiteral("bash"));
    process.setArguments({QStringLiteral("-c"), command});
    process.setProcessChannelMode(QProcess::MergedChannels);

    kdk::KDialog *progressDialog = m_progressDialog;
    QLabel *progressStatus = m_progressStatus;
    QPlainTextEdit *progressLog = m_progressLog;
    QString outputText;
    if (!m_progressSessionActive) {
        m_progressCancelled = false;
    }

    if ((!progressTitle.isEmpty() || !progressText.isEmpty()) && !progressDialog) {
        auto *kylinProgressDialog = new kdk::KDialog(QApplication::activeWindow());
        progressDialog = kylinProgressDialog;
        progressDialog->setWindowTitle(progressTitle.isEmpty() ? tr("请稍候") : progressTitle);
        progressDialog->setModal(true);
        progressDialog->resize(720, 420);

        auto *layout = new QVBoxLayout(kylinProgressDialog->mainWidget());
        progressStatus = new QLabel(progressDialog);
        layout->addWidget(progressStatus);

        progressLog = new QPlainTextEdit(progressDialog);
        progressLog->setReadOnly(true);
        progressLog->setLineWrapMode(QPlainTextEdit::NoWrap);
        layout->addWidget(progressLog);

        auto *buttonLayout = new QHBoxLayout();
        buttonLayout->addStretch(1);
        QPushButton *cancelButton = new QPushButton(tr("取消"), progressDialog);
        buttonLayout->addWidget(cancelButton);
        layout->addLayout(buttonLayout);
        connect(cancelButton, &QPushButton::clicked, progressDialog, [this, cancelButton]() {
            m_progressCancelled = true;
            m_startupRuntimeUserCancelled = true;
            cancelButton->setEnabled(false);
            if (m_progressStatus) {
                m_progressStatus->setText(tr("正在取消..."));
            }
        });

        progressStatus->setText(progressText.isEmpty() ? tr("请稍候") : progressText);
        progressDialog->show();
        QApplication::processEvents();
    } else if (progressLog || progressStatus) {
        if (progressStatus && !progressText.isEmpty()) {
            progressStatus->setText(progressText);
        }
        if (progressLog && !progressLog->document()->isEmpty()) {
            progressLog->appendPlainText(QString());
            progressLog->appendPlainText(QString());
        }
        QApplication::processEvents();
    }

    auto updateStatus = [&](qint64 elapsedMs) {
        if (!progressStatus || progressText.isEmpty()) {
            return;
        }
        const int dotCount = static_cast<int>((elapsedMs / 350) % 3) + 1;
        QString statusText = progressText + QString(dotCount, QLatin1Char('.'));
        progressStatus->setText(statusText);
    };

    auto appendOutput = [&]() {
        const QString chunk = QString::fromUtf8(process.readAll());
        if (chunk.isEmpty()) {
            return;
        }
        outputText += chunk;
        if (progressLog) {
            QScrollBar *bar = progressLog->verticalScrollBar();
            const bool followTail = !bar || bar->value() >= bar->maximum() - 2;
            QTextCursor cursor = progressLog->textCursor();
            cursor.movePosition(QTextCursor::End);
            cursor.insertText(chunk);
            if (followTail) {
                progressLog->verticalScrollBar()->setValue(progressLog->verticalScrollBar()->maximum());
            }
        }
    };

    process.start();

    QElapsedTimer timer;
    timer.start();
    bool timedOut = false;
    bool cancelled = false;
    updateStatus(0);
    while (process.state() != QProcess::NotRunning) {
        if (m_progressCancelled) {
            cancelled = true;
            break;
        }
        if (timeoutMs > 0 && timer.elapsed() > timeoutMs) {
            timedOut = true;
            break;
        }
        process.waitForReadyRead(100);
        appendOutput();
        updateStatus(timer.elapsed());
        QApplication::processEvents();
    }
    appendOutput();

    if (timedOut) {
        process.kill();
        process.waitForFinished(1000);
        appendOutput();
        if (output) {
            *output = outputText.trimmed();
        }
        if (progressLog) {
            progressLog->appendPlainText(QString());
            progressLog->appendPlainText(tr("操作超时。"));
        }
        if (shouldSaveRuntimeUpdateLog && progressLog) {
            saveRuntimeUpdateLog(progressLog->toPlainText());
        }
        if (progressDialog && !m_progressSessionActive) {
            progressDialog->deleteLater();
        }
        return false;
    }

    if (cancelled) {
        process.kill();
        process.waitForFinished(1000);
        appendOutput();
        if (output) {
            *output = outputText.trimmed();
        }
        if (progressLog) {
            progressLog->appendPlainText(QString());
            progressLog->appendPlainText(tr("操作已取消。"));
        }
        if (shouldSaveRuntimeUpdateLog && progressLog) {
            saveRuntimeUpdateLog(progressLog->toPlainText());
        }
        if (progressDialog && !m_progressSessionActive) {
            progressDialog->deleteLater();
        }
        return false;
    }

    const QString text = outputText.trimmed();
    if (output) {
        *output = text;
    }
    const bool ok = process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
    if (progressLog) {
        progressLog->appendPlainText(QString());
        progressLog->appendPlainText(ok ? tr("操作完成。") : tr("操作失败。"));
    }
    if (shouldSaveRuntimeUpdateLog && progressLog) {
        saveRuntimeUpdateLog(progressLog->toPlainText());
    }
    if (progressDialog && !m_progressSessionActive) {
        QApplication::processEvents();
        QThread::msleep(ok ? 500 : 1200);
        progressDialog->deleteLater();
    }
    return ok;
}

bool GatewayService::confirmAction(const QString &title, const QString &text, const QString &acceptText) const
{
    return showMessageDialog(QApplication::activeWindow(), title, text, acceptText, tr("取消"));
}

bool GatewayService::confirmRetryStartupRuntime() const
{
    return showMessageDialog(
        QApplication::activeWindow(),
        tr("安装/更新失败"),
        tr("智能体运行时安装或更新失败，可能是网络异常或仓库克隆失败。\n\n是否重试？"),
        tr("重试"),
        tr("取消"));
}

void GatewayService::beginProgressSession(const QString &title, const QString &text) const
{
    if (m_progressDialog) {
        return;
    }

    m_progressSessionActive = true;
    m_progressCancelled = false;
    m_progressRetryRequested = false;
    auto *kylinProgressDialog = new kdk::KDialog(QApplication::activeWindow());
    m_progressDialog = kylinProgressDialog;
    m_progressDialog->setWindowTitle(title);
    m_progressDialog->setModal(true);
    m_progressDialog->resize(720, 420);

    auto *layout = new QVBoxLayout(kylinProgressDialog->mainWidget());
    m_progressStatus = new QLabel(m_progressDialog);
    layout->addWidget(m_progressStatus);

    m_progressLog = new QPlainTextEdit(m_progressDialog);
    m_progressLog->setReadOnly(true);
    m_progressLog->setLineWrapMode(QPlainTextEdit::NoWrap);
    layout->addWidget(m_progressLog);

    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch(1);
    m_progressCancelButton = new QPushButton(tr("取消"), m_progressDialog);
    buttonLayout->addWidget(m_progressCancelButton);
    layout->addLayout(buttonLayout);
    connect(m_progressCancelButton, &QPushButton::clicked, m_progressDialog, [this]() {
        m_progressCancelled = true;
        m_startupRuntimeUserCancelled = true;
        if (m_progressCancelButton) {
            m_progressCancelButton->setEnabled(false);
        }
        if (m_progressStatus) {
            m_progressStatus->setText(tr("正在取消..."));
        }
    });

    if (!text.isEmpty()) {
        m_progressStatus->setText(text);
    }

    m_progressDialog->show();
    QApplication::processEvents();
}

void GatewayService::endProgressSession(bool ok) const
{
    if (!m_progressDialog) {
        m_progressSessionActive = false;
        return;
    }

    if (m_progressStatus) {
        m_progressStatus->setText(ok ? tr("全部操作完成。") : tr("操作失败。"));
    }
    if (m_progressCancelButton) {
        m_progressCancelButton->setEnabled(false);
    }
    if (m_progressLog && !m_progressLog->document()->isEmpty()) {
        m_progressLog->appendPlainText(QString());
    }
    if (m_progressLog) {
        m_progressLog->appendPlainText(ok ? tr("全部操作完成。") : tr("操作失败。"));
        saveRuntimeUpdateLog(m_progressLog->toPlainText());
    }

    QApplication::processEvents();

    if (!ok) {
        if (m_progressLog) {
            m_progressLog->appendPlainText(tr("请选择“重试”重新安装，或选择“退出”关闭此窗口。"));
        }
        if (m_progressCancelButton) {
            QObject::disconnect(m_progressCancelButton, nullptr, m_progressDialog, nullptr);
            m_progressCancelButton->setText(tr("退出"));
            m_progressCancelButton->setEnabled(true);

            QPushButton *retryButton = new QPushButton(tr("重试"), m_progressDialog);
            if (QLayout *rootLayout = m_progressDialog->layout()) {
                QLayoutItem *lastItem = rootLayout->itemAt(rootLayout->count() - 1);
                if (lastItem && lastItem->layout()) {
                    lastItem->layout()->addWidget(retryButton);
                }
            }

            QEventLoop loop;
            bool retryRequested = false;
            QObject::connect(retryButton, &QPushButton::clicked, &loop, [&]() {
                retryRequested = true;
                loop.quit();
            });
            QObject::connect(m_progressCancelButton, &QPushButton::clicked, &loop, &QEventLoop::quit);
            QObject::connect(m_progressDialog, &kdk::KDialog::finished, &loop, &QEventLoop::quit);
            loop.exec();
            m_progressRetryRequested = retryRequested;
            m_startupRuntimeUserCancelled = !retryRequested;
        }
    } else {
        QThread::msleep(700);
    }

    m_progressDialog->deleteLater();
    m_progressDialog = nullptr;
    m_progressStatus = nullptr;
    m_progressLog = nullptr;
    m_progressCancelButton = nullptr;
    m_progressSessionActive = false;
    m_progressCancelled = false;
}

void GatewayService::saveRuntimeUpdateLog(const QString &text) const
{
    const QString trimmedText = text.trimmed();
    if (trimmedText.isEmpty()) {
        return;
    }

    const QString dirPath = QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation))
        .filePath(QStringLiteral(".local/share/kylin-agent"));
    QDir().mkpath(dirPath);

    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz"));
    const QString path = QDir(dirPath).filePath(QStringLiteral("update-log.%1.log").arg(timestamp));
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "Failed to open runtime update log for writing:" << path << file.errorString();
        return;
    }

    QTextStream out(&file);
    out << trimmedText << '\n';
    if (!file.commit()) {
        qWarning() << "Failed to save runtime update log:" << path << file.errorString();
        return;
    }

    qDebug() << "Saved runtime update log:" << path;
}

QString GatewayService::readTrimmedFile(const QString &path) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    return QString::fromUtf8(file.readAll()).trimmed();
}
