#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QPointer>
#include <QRect>

class QSettings;

class Config : public QObject
{
    Q_OBJECT
public:
    static Config &getInstance();
    // config
    QString getTitle();
    QString getServerVersion();
    int getMaxFps();
    int getDesktopOpenGL();
    int getRenderExpiredFrames();
    QString getPushFilePath();
    QString getServerPath();
    QString getAdbPath();
    QString getLogLevel();
    QString getCodecOptions();
    QString getCodecName();
    QString getUserName();
    QString getPassWord();
    void setUserName(const QString &userName);
    void setPassword(const QString &password);
    QStringList getConnectedGroups();
    void setRect(const QString &serial, const QRect &rc);
    QRect getRect(const QString &serial);
    void deleteGroup(const QString &serial);
    int  loadDistance();
    void updateDistance(const int distance);

private:
    explicit Config(QObject *parent = nullptr);
    const QString &getConfigPath();

private:
    static QString s_configPath;
    QPointer<QSettings> m_settings; // 模板类 ， 释放引用对象后自动置null
    QPointer<QSettings> m_userData;
    QPointer<QSettings> m_password;
    QPointer<QSettings> m_scrollSet;
};

#endif // CONFIG_H
