#ifndef __KOM_CONFIGURE_H__
#define __KOM_CONFIGURE_H__

#include <memory>

#include <QVariant>
#include <QString>

#define CONFIG_PLUGIN_PROBLEM_FEEDBACK "ProblemFeedback"
#define CONFIG_PLUGIN_PROBLEM_FEEDBACK_NAME_ADDRESS "NameAddress"
#define CONFIG_PLUGIN_PROBLEM_FEEDBACK_EMAIL "Email"
#define CONFIG_PLUGIN_PROBLEM_FEEDBACK_CONTACT "ContactInformation"
#define CONFIG_PLUGIN_PROBLEM_FEEDBACK_PROTOCOL "Protocol"
#define CONFIG_PLUGIN_PROBLEM_FEEDBACK_DOMAIN "Domain"
#define CONFIG_PLUGIN_PROBLEM_FEEDBACK_PORT "Port"
#define CONFIG_PLUGIN_PROBLEM_FEEDBACK_ENABLE_UPLOAD "EnableUpload"
#define CONFIG_PLUGIN_PROBLEM_FEEDBACK_EXPORT_PATH "ExportPath"
#define CONFIG_PLUGIN_PROBLEM_FEEDBACK_HISTORY_BUG "HistoryBug"
#define CONFIG_PLUGIN_PROBLEM_FEEDBACK_HISTORY_BUG_KEY "HistoryBugKey"

#define CONFIG_PLUGIN_GENERAL_NAME "Name"
#define CONFIG_PLUGIN_GENERAL_ICON "Icon"

#define CONFIG_CHEST_WIN_DATA_MIGRATION "WinDataMigration"
#define CONFIG_CHEST_WIN_DATA_MIGRATION_FIRST_OPEN "FirstOpen"
#define CONFIG_CHEST_WIN_DATA_MIGRATION_READ_DIR_CHILD "ReadDirChild"
#define CONFIG_CHEST_WIN_DATA_MIGRATION_DEBUG_LEVEL "DebugLevel"


#define CONFIG_DAEMON_SYSTEM_MONITOR "SystemMonitor"
#define CONFIG_DAEMON_SYSTEM_MONITOR_CPU "CpuThreshold"
#define CONFIG_DAEMON_SYSTEM_MONITOR_MEMORY "MemoryThreshold"
#define CONFIG_DAEMON_SYSTEM_MONITOR_TIMER "Timer"
#define CONFIG_DAEMON_SYSTEM_MONITOR_OUTPUT "Output"

#define CONFIG_PLUGIN_REPLACEMENT_ASSISTANT "ReplacementAssistant"
#define CONFIG_PLUGIN_REPLACEMENT_ASSISTANT_CODE "Code"

namespace kom
{

class Configure
{
public:
    Configure();
    ~Configure();

    QVariant value(const QString &group, const QString &key, const QVariant &defaultValue = QVariant());
    void setValue(const QString &group, const QString &key, const QVariant &value);

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};
} // namespace kom

#endif
