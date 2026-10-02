#include <QCoreApplication>
#include <QFileInfo>
#include <QSettings>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>

#include "config.h"

#define GROUP_COMMON "common"

// config
#define COMMON_TITLE_KEY "WindowTitle"
#define COMMON_TITLE_DEF QCoreApplication::applicationName()

#define COMMON_PUSHFILE_KEY "PushFilePath"
#define COMMON_PUSHFILE_DEF "/sdcard/"

#define COMMON_SERVER_VERSION_KEY "ServerVersion"
#define COMMON_SERVER_VERSION_DEF "1.21"

#define COMMON_SERVER_PATH_KEY "ServerPath"
#define COMMON_SERVER_PATH_DEF "/data/local/tmp/scrcpy-server.jar"

#define COMMON_MAX_FPS_KEY "MaxFps"
#define COMMON_MAX_FPS_DEF 60

#define COMMON_DESKTOP_OPENGL_KEY "UseDesktopOpenGL"
#define COMMON_DESKTOP_OPENGL_DEF -1

#define COMMON_RENDER_EXPIRED_FRAMES_KEY "RenderExpiredFrames"
#define COMMON_RENDER_EXPIRED_FRAMES_DEF 0

#define COMMON_ADB_PATH_KEY "AdbPath"
#define COMMON_ADB_PATH_DEF ""

#define COMMON_LOG_LEVEL_KEY "LogLevel"
#define COMMON_LOG_LEVEL_DEF "info"

#define COMMON_CODEC_OPTIONS_KEY "CodecOptions"
#define COMMON_CODEC_OPTIONS_DEF ""

#define COMMON_CODEC_NAME_KEY "CodecName"
#define COMMON_CODEC_NAME_DEF ""

#define COMMON_USERNAME_KEY "UserName"
#define COMMON_USERNAME_DEF "admin"

#define COMMON_PASSWORD_KEY "Password"
#define COMMON_PASSWORD_DEF "admin"

#define DEFAULT_SCROLL_KEY "Distance"
#define DEFAULT_SCROLL_DEF 40

// device config
#define SERIAL_WINDOW_RECT_KEY_X "WindowRectX"
#define SERIAL_WINDOW_RECT_KEY_Y "WindowRectY"
#define SERIAL_WINDOW_RECT_KEY_W "WindowRectW"
#define SERIAL_WINDOW_RECT_KEY_H "WindowRectH"
#define SERIAL_WINDOW_RECT_KEY_DEF -1

QString Config::s_configPath = "";

Config::Config(QObject *parent) : QObject(parent)
{
    const QString &configPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config";
    if (!QDir(configPath).exists("kylin-connectivity")) {
        QDir(configPath).mkdir("kylin-connectivity");
    }
    QString setName = QDir::homePath() + "/.config/kylin-connectivity/phoneServerConfig.ini";
    m_settings = new QSettings(setName, QSettings::IniFormat);
    m_settings->setIniCodec("UTF-8");

    QString userDataName = QDir::homePath() + "/.config/kylin-connectivity/phoneDeviceConfig.ini";
    m_userData = new QSettings(userDataName, QSettings::IniFormat);
    m_userData->setIniCodec("UTF-8");

    QString scrollSetName = QDir::homePath() + "/.config/kylin-connectivity/scrollConfig.ini";
    m_scrollSet = new QSettings(scrollSetName, QSettings::IniFormat);
    m_scrollSet->setIniCodec("UTF-8");

    m_password = new QSettings(getenv("HOME") + QString("/.connectivitycache/setting.ini"), QSettings::IniFormat);
    m_password->setIniCodec("UTF-8");
}

Config &Config::getInstance()
{
    static Config config;
    return config;
}

const QString &Config::getConfigPath()
{
    if (s_configPath.isEmpty()) {
        s_configPath = QString::fromLocal8Bit(qgetenv("CONFIG_PATH"));
        QFileInfo fileInfo(s_configPath);
        if (s_configPath.isEmpty() || !fileInfo.isDir()) {
            // default application dir
            s_configPath = "config";
        }
    }
    return s_configPath;
}

void Config::setRect(const QString &serial, const QRect &rc)
{
    m_userData->beginGroup(serial);
    m_userData->setValue(SERIAL_WINDOW_RECT_KEY_X, rc.left());
    m_userData->setValue(SERIAL_WINDOW_RECT_KEY_Y, rc.top());
    m_userData->setValue(SERIAL_WINDOW_RECT_KEY_W, rc.width());
    m_userData->setValue(SERIAL_WINDOW_RECT_KEY_H, rc.height());
    m_userData->endGroup();
    m_userData->sync();
}

QRect Config::getRect(const QString &serial)
{
    QRect rc;
    m_userData->beginGroup(serial);
    rc.setX(m_userData->value(SERIAL_WINDOW_RECT_KEY_X, SERIAL_WINDOW_RECT_KEY_DEF).toInt());
    rc.setY(m_userData->value(SERIAL_WINDOW_RECT_KEY_Y, SERIAL_WINDOW_RECT_KEY_DEF).toInt());
    rc.setWidth(m_userData->value(SERIAL_WINDOW_RECT_KEY_W, SERIAL_WINDOW_RECT_KEY_DEF).toInt());
    rc.setHeight(m_userData->value(SERIAL_WINDOW_RECT_KEY_H, SERIAL_WINDOW_RECT_KEY_DEF).toInt());
    m_userData->endGroup();
    return rc;
}


QString Config::getServerVersion()
{
    QString server;
    m_settings->beginGroup(GROUP_COMMON);
    server = m_settings->value(COMMON_SERVER_VERSION_KEY, COMMON_SERVER_VERSION_DEF).toString();
    m_settings->endGroup();
    return server;
}

int Config::getMaxFps()
{
    int fps = 60;
    m_settings->beginGroup(GROUP_COMMON);
    fps = m_settings->value(COMMON_MAX_FPS_KEY, COMMON_MAX_FPS_DEF).toInt();
    m_settings->endGroup();
    return fps;
}

int Config::getDesktopOpenGL()
{
    int opengl = 0;
    m_settings->beginGroup(GROUP_COMMON);
    opengl = m_settings->value(COMMON_DESKTOP_OPENGL_KEY, COMMON_DESKTOP_OPENGL_DEF).toInt();
    m_settings->endGroup();
    return opengl;
}

int Config::getRenderExpiredFrames()
{
    int renderExpiredFrames = 1;
    m_settings->beginGroup(GROUP_COMMON);
    renderExpiredFrames = m_settings->value(COMMON_RENDER_EXPIRED_FRAMES_KEY, COMMON_RENDER_EXPIRED_FRAMES_DEF).toInt();
    m_settings->endGroup();
    return renderExpiredFrames;
}

QString Config::getPushFilePath()
{
    QString pushFile;
    m_settings->beginGroup(GROUP_COMMON);
    pushFile = m_settings->value(COMMON_PUSHFILE_KEY, COMMON_PUSHFILE_DEF).toString();
    m_settings->endGroup();
    return pushFile;
}

QString Config::getServerPath()
{
    QString serverPath;
    m_settings->beginGroup(GROUP_COMMON);
    serverPath = m_settings->value(COMMON_SERVER_PATH_KEY, COMMON_SERVER_PATH_DEF).toString();
    m_settings->endGroup();
    return serverPath;
}

QString Config::getAdbPath()
{
    QString adbPath;
    m_settings->beginGroup(GROUP_COMMON);
    adbPath = m_settings->value(COMMON_ADB_PATH_KEY, COMMON_ADB_PATH_DEF).toString();
    m_settings->endGroup();
    return adbPath;
}

QString Config::getLogLevel()
{
    QString logLevel;
    m_settings->beginGroup(GROUP_COMMON);
    logLevel = m_settings->value(COMMON_LOG_LEVEL_KEY, COMMON_LOG_LEVEL_DEF).toString();
    m_settings->endGroup();
    return logLevel;
}

QString Config::getCodecOptions()
{
    QString codecOptions;
    m_settings->beginGroup(GROUP_COMMON);
    codecOptions = m_settings->value(COMMON_CODEC_OPTIONS_KEY, COMMON_CODEC_OPTIONS_DEF).toString();
    m_settings->endGroup();
    return codecOptions;
}

QString Config::getCodecName()
{
    QString codecName;
    m_settings->beginGroup(GROUP_COMMON);
    codecName = m_settings->value(COMMON_CODEC_NAME_KEY, COMMON_CODEC_NAME_DEF).toString();
    m_settings->endGroup();
    return codecName;
}

QStringList Config::getConnectedGroups()
{
    return m_userData->childGroups();
}

void Config::deleteGroup(const QString &serial)
{
    m_userData->remove(serial);
}

QString Config::getTitle()
{
    QString title;
    m_settings->beginGroup(GROUP_COMMON);
    title = m_settings->value(COMMON_TITLE_KEY, COMMON_TITLE_DEF).toString();
    m_settings->endGroup();
    return title;
}

QString Config::getUserName()
{
    QString userName;
    userName = m_password->value(COMMON_USERNAME_KEY, COMMON_USERNAME_DEF).toString();
    return userName;
}

QString Config::getPassWord()
{
    QString password;
    password = m_password->value(COMMON_PASSWORD_KEY, COMMON_PASSWORD_DEF).toString();
    return password;
}

void Config::setUserName(const QString &username)
{
    m_password->setValue(COMMON_USERNAME_KEY, username);
    m_password->sync();
}

void Config::setPassword(const QString &password)
{
    m_password->setValue(COMMON_PASSWORD_KEY, password);
    m_password->sync();
}

int Config::loadDistance()
{
    m_scrollSet->beginGroup(DEFAULT_SCROLL_KEY);
    int distance = m_scrollSet->value(DEFAULT_SCROLL_KEY, DEFAULT_SCROLL_DEF).toInt();
    m_scrollSet->endGroup();
    return distance;
}

void Config::updateDistance(const int distance)
{
    m_scrollSet->beginGroup(DEFAULT_SCROLL_KEY);
    m_scrollSet->setValue(DEFAULT_SCROLL_KEY, distance);
    m_scrollSet->endGroup();
    m_scrollSet->sync();
}
