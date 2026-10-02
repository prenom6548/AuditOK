#include "feedbackcommon.h"
#include <pwd.h>
#include <unistd.h>
#include <QDebug>
#include <QStandardPaths>
#include <QFile>
#include "kom-configure.h"
#include "src/settings.h"

void FeedBackCommon::setArgs(QString path, QString exe, QString restart, QString mode, QString code)
{
    m_path = path;
    m_exe = exe;
    m_resart = restart;
    m_mode = mode;
    m_code = code;
    m_datetime = QString::number(QDateTime::currentDateTime().toSecsSinceEpoch());
    Settings::Customize customize = Settings::getCustomize();
    if (customize == Settings::INTERNAL) {
        m_internalMode = ProjectMode;
    } else if (customize == Settings::ICBC) {
        m_internalMode = ProjectMode;
    } else {
        m_internalMode = DefaultMode;
    }
}

void FeedBackCommon::startCollecting(bool agree ,QString text ,QString phoneNumber ,int score)
{
    kom::Configure conf;
    //配置采集内容
    InformationClassItemList classList = FeedbackManager::getInstance()->getLogFileItems();
    for (InformationClassItem *item : classList) {
        if (item->getItemName() == "整机信息" || item->getItemName() == "硬件参数" || item->getItemName() == "网络状态") {
            if (agree) {
                continue;
            }
        }
        item->setSelect(false);
    }
    //设置提交内容
    InformationUserStruct info;
    info.module = m_internalMode;
    if (phoneNumber.isEmpty()) {
        phoneNumber = conf.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_CONTACT).toString();
    } else {
        //保存配置
        conf.setValue(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_CONTACT, phoneNumber);
    }
    if (phoneNumber.isEmpty()) {
        phoneNumber = "11111111111";
    }
    info.phoneNumber = phoneNumber;
    info.user = conf.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_NAME_ADDRESS).toString();
    if (info.user.isEmpty()) {
        info.user = "匿名";
    }
    info.serviceNumber = Settings::getServiceNumber();
    struct passwd *pw = getpwuid(getuid());
    QString userName;
    if (pw) {
        userName = QString(pw->pw_gecos);
        info.jobNumber = QString(pw->pw_name);
    }
    info.userName = userName;

    // 因工单系统显示的是内容而不是标题，所以往内容的头部插入一行文本来充当标题
    QString content_header;
    QString title = "【管家反馈】";
    if (m_mode == "1") {
        // 主动反馈
        info.priviteType = "3-0";
        info.priviteTypeStr = "应用软件";
        title += "应用软件";
        title += "|" + m_exe + "|" + QString::number(score) + "星";
        if (!text.isEmpty()) {
            title += "|" + text.split("\n").first().mid(0, 20);
        }
        content_header = QString("【主动反馈】【应用软件】【%1】").arg(m_exe);
    } else if (m_mode == "2") {
        // 被动反馈
        info.debugFilePath = m_path;
        info.priviteType = "1-3";
        info.priviteTypeStr = "系统使用，系统崩溃";
        title += "系统崩溃|"+m_exe;
        content_header = QString("【被动反馈】【系统崩溃】【%1】").arg(m_exe);
    } else if (m_mode == "3") {
        // 管家故障诊断调用
        info.debugFilePath = m_path;
        info.priviteType = "1-2";
        info.priviteTypeStr = "系统使用，系统安装";
        title += "故障修复|"+m_exe;
        if (!m_code.isEmpty()) {
           title+="|"+ m_code;
        }
        content_header = QString("【故障诊断】【故障修复】【%1】【%2】").arg(m_exe, m_code);
    }
    info.uniqueType =  QString::number(getpid()) + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
    if (m_internalMode == ProjectMode) {
        //禅道短时间创建标题名重复的bug会被拦截
        //采用用户名+随机数的方式区分
         title +="|";
        if (!userName.isEmpty()) {
           title +=userName + "|";
        }
        title += info.uniqueType;
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

QString FeedBackCommon::getAppLogFile()
{
    QString getLogFile = m_exe;
    if (getLogFile.contains("/")) {
        getLogFile = getLogFile.split("/").last();
    }
    getLogFile = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.log/" + getLogFile + ".log";
    qDebug()<<"自动添加日志文件"<<getLogFile;
    if (QFile(getLogFile).exists()) {
        return getLogFile;
    }
    return "";
}
