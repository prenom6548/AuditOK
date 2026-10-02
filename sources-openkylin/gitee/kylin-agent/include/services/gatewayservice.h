#ifndef GATEWAYSERVICE_H
#define GATEWAYSERVICE_H

#include <QObject>
#include <QString>

class QProcess;
class QLabel;
class QPlainTextEdit;
class QPushButton;
namespace kdk { class KDialog; }

class GatewayService : public QObject
{
    Q_OBJECT

public:
    explicit GatewayService(QObject *parent = nullptr);
    ~GatewayService();

    static GatewayService *activeInstance();
    bool ensureReady();
    bool restartGateway();

public slots:
    void stopManagedGateway();

private:
    QString runtimeExecutable() const;
    QString runtimeHome() const;
    QString configPath() const;
    QString runtimeRoot() const;
    QString cuaRuntimePath() const;
    QString cuaRuntimeSourcePath() const;
    QString cuaSkillSourcePath() const;
    QString cuaSkillInstallPath() const;
    QString runtimeSkillsInstallPath() const;
    QString runtimeSkillsSourcePath() const;
    QString runtimeInstallPath() const;
    QString runtimeCacheSourcePath() const;
    QString runtimeCacheMarkerPath() const;
    QString gatewayServicePath() const;
    QString gatewayServiceDefinition() const;
    QString versionFilePath(const QString &directory) const;
    bool hasRuntimeExecutable() const;
    bool hasBackendRuntime() const;
    bool hasRuntimeVenv() const;
    bool hasCompleteRuntime() const;
    bool hasRuntimeCachePackage() const;
    bool hasRuntimeCacheInstalled() const;
    bool ensureRuntimeLimitsConfig();
    bool ensureApiServerConfig();
    bool ensureRuntimeSkillsSeeded();
    bool ensureStartupRuntimes(bool gatewayWasRunning, bool skipConfirm = false);
    bool ensureRuntimeInstalled();
    bool runRuntimeInstaller();
    bool runRuntimeCacheInstaller();
    bool updateDefaultSoulMd() const;
    bool ensureCuaRuntime();
    bool ensureYdotoolEnvironment();
    bool hasYdotoolEnvironment() const;
    bool ensureBackendRuntime(bool gatewayWasRunning, bool promptBeforeUpdate = true);
    bool needsCuaRuntimeUpdate() const;
    bool needsCuaSkillSync() const;
    bool needsBackendRuntimeUpdate() const;
    bool updateCuaRuntime();
    bool syncCuaSkill();
    bool isApiServerAvailable() const;
    bool isGatewayRunning() const;
    bool isGatewayServiceActive() const;
    bool ensureGatewayServicesInstalled();
    bool disableLegacyGatewayServices();
    bool runSystemctl(const QStringList &arguments, QString *output = nullptr) const;
    bool stopGateway();
    bool startGateway();
    bool runRuntimeCommand(const QStringList &arguments, QString *output = nullptr) const;
    bool runShellCommand(const QString &command,
                         int timeoutMs,
                         QString *output = nullptr,
                         const QString &progressTitle = QString(),
                         const QString &progressText = QString(),
                         bool shouldSaveRuntimeUpdateLog = false) const;
    bool confirmAction(const QString &title, const QString &text, const QString &acceptText) const;
    bool confirmRetryStartupRuntime() const;
    void beginProgressSession(const QString &title, const QString &text) const;
    void endProgressSession(bool ok) const;
    void saveRuntimeUpdateLog(const QString &text) const;
    QString readTrimmedFile(const QString &path) const;

    QProcess *m_gatewayProcess;
    mutable kdk::KDialog *m_progressDialog;
    mutable QLabel *m_progressStatus;
    mutable QPlainTextEdit *m_progressLog;
    mutable QPushButton *m_progressCancelButton;
    mutable bool m_progressSessionActive;
    mutable bool m_progressCancelled;
    mutable bool m_progressRetryRequested;
    mutable bool m_startupRuntimeUserCancelled;
};

#endif // GATEWAYSERVICE_H
