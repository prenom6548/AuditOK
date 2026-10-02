#ifndef HERMESGATEWAYSERVICE_H
#define HERMESGATEWAYSERVICE_H

#include <QObject>
#include <QString>

class QProcess;
class QDialog;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class GatewayService;

class HermesGatewayService : public QObject
{
    Q_OBJECT

public:
    explicit HermesGatewayService(QObject *parent = nullptr);
    ~HermesGatewayService();

    static HermesGatewayService *activeInstance();
    bool ensureReady();
    bool restartGateway();

public slots:
    void stopManagedGateway();

private:
    QString hermesExecutable() const;
    QString hermesHome() const;
    QString configPath() const;
    QString runtimeRoot() const;
    QString runtimeSourcePath() const;
    QString cuaRuntimePath() const;
    QString cuaSkillSourcePath() const;
    QString cuaSkillImportPath() const;
    QString agentBackendPath() const;
    QString versionFilePath(const QString &directory) const;
    bool hasHermesExecutable() const;
    bool hasHermesBackendRuntime() const;
    bool hasCompleteHermesRuntime() const;
    bool ensureApiServerConfig();
    bool ensureStartupRuntimes(bool gatewayWasRunning);
    bool ensureHermesInstalled();
    bool ensureCuaRuntime();
    bool ensureBackendRuntime(bool gatewayWasRunning, bool promptBeforeUpdate = true);
    bool needsCuaRuntimeUpdate() const;
    bool needsCuaSkillSync() const;
    bool needsBackendRuntimeUpdate() const;
    bool updateRuntimeSource();
    bool updateCuaRuntime();
    bool syncCuaSkill();
    bool syncBackendRuntime();
    bool isApiServerAvailable() const;
    bool isGatewayRunning() const;
    bool stopGateway();
    bool startGateway();
    bool runHermesCommand(const QStringList &arguments, QString *output = nullptr) const;
    bool runShellCommand(const QString &command,
                         int timeoutMs,
                         QString *output = nullptr,
                         const QString &progressTitle = QString(),
                         const QString &progressText = QString()) const;
    bool confirmAction(const QString &title, const QString &text, const QString &acceptText) const;
    bool confirmRetryStartupRuntime() const;
    void beginProgressSession(const QString &title, const QString &text) const;
    void endProgressSession(bool ok) const;
    QString readTrimmedFile(const QString &path) const;

    QProcess *m_gatewayProcess;
    mutable QDialog *m_progressDialog;
    mutable QLabel *m_progressStatus;
    mutable QPlainTextEdit *m_progressLog;
    mutable QPushButton *m_progressCancelButton;
    mutable bool m_progressSessionActive;
    mutable bool m_progressCancelled;
    mutable bool m_startupRuntimeUserCancelled;
    GatewayService *m_gatewayService;
};

#endif // HERMESGATEWAYSERVICE_H
