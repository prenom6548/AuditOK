#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <QObject>
#include <QDebug>
#include <QDBusInterface>
#include <QDBusReply>
#include "QGSettings/qgsettings.h"

extern "C" {
#include <glib.h>
}

typedef struct _UserInfomation {
    QString objpath;
    QString username;
    QString realname;
    QString iconfile;
    QString passwd;
    int accounttype;
    int passwdtype;
    bool current;
    bool logined;
    bool autologin;
    bool noPwdLogin;
    qint64 uid;
}UserInfomation;


class processmanager :public QObject
{
    Q_OBJECT
public:
    processmanager();
    Q_INVOKABLE QString getUserName();
    UserInfomation GetUserInformation(QString objpath);
    Q_INVOKABLE void startAiAssistant();
    Q_INVOKABLE bool isOpenkylin();
    Q_INVOKABLE QString getThemeStyle();
signals:
    Q_INVOKABLE void styleNameChanged(QString);
private:
    void runCommand(const QString &command);
private:
    QGSettings *m_styleSettings = nullptr;
};

#endif // PROCESSMANAGER_H
