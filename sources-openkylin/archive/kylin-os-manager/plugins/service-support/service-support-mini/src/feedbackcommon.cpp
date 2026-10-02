#include "feedbackcommon.h"
#include <pwd.h>
#include <unistd.h>
#include <glib.h>
#include <QDebug>
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>
#include "kom-configure.h"
#include "src/settings.h"

FeedBackCommon::FeedBackCommon()
    : annex_size_max_limit_(10 * 1024 * 1024),    // 10M
      m_internalMode(InternalMode::OpenKylin)
{
    exec_desktop_map_ = {
        {"kybackup", "yhkylin-backup-tools"},
        {"kylin-log-viewer", "logview"}
    };
}

void FeedBackCommon::setArgs(QString path, QString exe, QString restart, QString mode, QString code)
{
    m_path = path;
    m_exe = exe;
    m_resart = restart;
    m_mode = mode;
    m_code = code;
    m_datetime = QString::number(QDateTime::currentDateTime().toSecsSinceEpoch());
}

void FeedBackCommon::startCollecting(bool agree, QString text, QString phoneNumber, int score) {
    kom::Configure conf;
    // 配置采集内容
    InformationClassItemList classList = FeedbackManager::getInstance()->getLogFileItems();
    for (InformationClassItem *item : classList) {
        if (agree) {
            item->setSelect(true);
        } else {
            item->setSelect(false);
        }
    }

    // 设置提交内容
    InformationUserStruct info;
    // 模式
    info.module = m_internalMode;
    // 手机号
    if (phoneNumber.isEmpty()) {
        phoneNumber = conf.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_CONTACT).toString();
    } else {
        conf.setValue(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_CONTACT, phoneNumber);
    }
    if (phoneNumber.isEmpty()) {
        phoneNumber = "11111111111";
    }
    info.phoneNumber = phoneNumber;

    // 称谓
    info.name = conf.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_NAME_ADDRESS).toString();
    if (info.name.isEmpty()) {
        info.name = "匿名";
    }

    // 因工单系统显示的是内容而不是标题，所以往内容的头部插入一行文本来充当标题
    QString content_header;
    QString title = "【管家反馈】";
    if (m_mode == "1") {
        // 主动反馈
        info.priviteType = "3-0";
        title += "应用软件";
        title += "|" + m_exe + "|" + QString::number(score) + "星";
        if (!text.isEmpty()) {
            title += "|" + text.split("\n").first().mid(0, 20);
        }
        content_header = QString("【主动反馈】【应用软件】【%1】").arg(m_exe);
    } else if (m_mode == "2") {
        // 被动反馈
        info.priviteType = "1-3";
        title += "系统崩溃|" + m_exe;
        content_header = QString("【被动反馈】【系统崩溃】【%1】").arg(m_exe);
    } else if (m_mode == "3") {
        // 管家故障诊断调用
        info.priviteType = "1-2";
        title += "故障修复|"+m_exe;
        if (!m_code.isEmpty()) {
           title+="|"+ m_code;
        }
        content_header = QString("【故障诊断】【故障修复】【%1】【%2】").arg(m_exe, m_code);
    }
    info.title = title;
    if (text.isEmpty()) {
        text = "未填写任何内容";
    }
    info.details = QString("%1\n%2").arg(content_header, text);

    FeedbackManager::getInstance()->setUserData(info);
    FeedbackManager::getInstance()->setSavePath("/tmp");

    //开始提交
    FeedbackManager::getInstance()->startCollecting();
}

QString FeedBackCommon::getAppLogFile() {
    QString getLogFile = m_exe;
    if (getLogFile.contains("/")) {
        getLogFile = getLogFile.split("/").last();
    }
    getLogFile = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.log/" + getLogFile + ".log";

    // 判断日志文件是否超出了最大限制
    QFileInfo file_info(getLogFile);
    if (!file_info.exists() ||
        file_info.size() > annex_size_max_limit_)
    {
        qWarning() << "Component log does not exist or exceeds size";
        return "";
    }

    qInfo() << "Add component log: " << getLogFile;

    return getLogFile;
}

QString FeedBackCommon::GetDesktopPath() const {
    QString desktop;
    const auto iter = exec_desktop_map_.find(m_exe);
    if (iter != exec_desktop_map_.end()) {
        desktop = QString("/usr/share/applications/%1.desktop").arg(iter.value());
    } else {
        desktop = QString("/usr/share/applications/%1.desktop").arg(m_exe);
    }
    return desktop;
}

QString FeedBackCommon::ExecName() const {
    const auto desktop_path = GetDesktopPath().toStdString();

    g_autoptr(GKeyFile) key_file = g_key_file_new();
    g_autoptr(GError) error = NULL;
    if (!g_key_file_load_from_file(key_file, desktop_path.c_str(), G_KEY_FILE_KEEP_TRANSLATIONS, &error)) {
        qCritical() << "Get exec name fail: load key file fail, " << error->message;
        return m_exe;
    }

    g_autofree gchar *name = g_key_file_get_locale_string(key_file, "Desktop Entry", "Name", NULL, &error);
    if (name != NULL) {
        return QString(name);
    }

    return m_exe;
}

QString FeedBackCommon::ExecIcon() const {
    const auto desktop_path = GetDesktopPath().toStdString();

    g_autoptr(GKeyFile) key_file = g_key_file_new();
    g_autoptr(GError) error = NULL;
    if (!g_key_file_load_from_file(key_file, desktop_path.c_str(), G_KEY_FILE_NONE, &error)) {
        qCritical() << "Get exec icon fail: load key file fail, " << error->message;
        return "";
    }

    g_autofree gchar *icon = g_key_file_get_string(key_file, "Desktop Entry", "Icon", &error);
    if (icon != NULL) {
        return QString(icon);
    }

    return "";
}
