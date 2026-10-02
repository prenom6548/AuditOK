#include "vadbusoperator.h"
#include <QDBusConnection>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

VADBusOperator::VADBusOperator(QObject *parent)
    : QObject{parent}
{
    QDir().mkpath(QFileInfo(pidFilePath()).absolutePath());
    QFile pidFile(pidFilePath());
    if (pidFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        pidFile.write(QByteArray::number(QCoreApplication::applicationPid()));
        pidFile.write("\n");
    }

    QDBusConnection::sessionBus().unregisterService("org.ukui.voice_assistant");
    QDBusConnection::sessionBus().registerService("org.ukui.voice_assistant");
    QDBusConnection::sessionBus().registerObject("/org/ukui/voice_assistant",
        "org.ukui.voice_assistant", this,
        QDBusConnection::ExportNonScriptableSlots | QDBusConnection::ExportAllSignals);
}

VADBusOperator::~VADBusOperator()
{
    QFile::remove(pidFilePath());
}

QString VADBusOperator::pidFilePath()
{
    const QString runtimeDir = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    if (!runtimeDir.isEmpty()) {
        return runtimeDir + QStringLiteral("/voice-assistant.pid");
    }
    return QDir::homePath() + QStringLiteral("/.cache/kylin-aiassistant/voice-assistant.pid");
}

void VADBusOperator::ShowWindow()
{
    Q_EMIT sigShowWindow();
}

void VADBusOperator::Quit()
{
    Q_EMIT sigQuit();
}

bool VADBusOperator::Ping()
{
    return true;
}

qlonglong VADBusOperator::ProcessId()
{
    return QCoreApplication::applicationPid();
}
