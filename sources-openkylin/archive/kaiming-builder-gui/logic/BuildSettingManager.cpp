#include <logic/BuildSettingManager.h>

#include <QStandardPaths>
#include <QDir>
#include <QSettings>
#include <QDebug>

BuildSettingManager::BuildSettingManager(QObject *parent) : QObject(parent)
{
    loadConfig();
}
QString BuildSettingManager::getConfigPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +
           "/.cache/kaiming-builder-gui/config";

}

void BuildSettingManager::loadConfig()
{
    QString configPath = getConfigPath();
    QDir().mkpath(QFileInfo(configPath).path());

    QSettings settings(configPath, QSettings::IniFormat);
    bool isFirstRun = !QFile::exists(configPath);

    m_buildOnly = settings.value("buildOnly", false).toBool();
    m_finishOnly = settings.value("finishOnly", false).toBool();
    m_exportOnly = settings.value("exportOnly", false).toBool();
    m_forceClean = settings.value("forceClean", false).toBool();

    // 如果是首次，默认开启 keepSource；否则读取用户设置
    m_keepSource = settings.value("keepSource", isFirstRun).toBool();
    m_keepSource = settings.value("forceClean", isFirstRun).toBool();
    if (isFirstRun) {
        saveConfig();
    }
}

void BuildSettingManager::saveConfig() const
{
    QString configPath = getConfigPath();
    QSettings settings(configPath, QSettings::IniFormat);

    settings.setValue("buildOnly", m_buildOnly);
    settings.setValue("finishOnly", m_finishOnly);
    settings.setValue("exportOnly", m_exportOnly);
    settings.setValue("forceClean", m_forceClean);
    settings.setValue("keepSource", m_keepSource);
}

QStringList BuildSettingManager::getCommandString() const
{
    QStringList commands;

    if (m_buildOnly) commands << "--build-only";
    if (m_finishOnly) commands << "--finish-only";
    if (m_exportOnly) commands << "--export-only";
    if (m_forceClean) commands << "-f";
    if (m_keepSource) commands << "-k";

    return commands;
}
