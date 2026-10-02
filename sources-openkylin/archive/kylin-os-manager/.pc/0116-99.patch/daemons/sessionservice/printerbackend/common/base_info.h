#ifndef BASEINFO_H
#define BASEINFO_H

#include <QObject>
#include <QString>
#include "singleton.h"

class BaseInfo : public QObject,
                 public Singleton<BaseInfo>
{
    Q_OBJECT
    friend class Singleton<BaseInfo>;
private:
    BaseInfo();
    ~BaseInfo() override;

    void init();
    void setSystemVersion();
public:
    QString getAppDisplayName();
    QString getDebianArchitecture();
    QString getServerAddress();
    QString getSystemVersion();
private:
    QString m_appDisplayName;
    QString m_debianArchitecture;
    QString m_serverAddress;
    QString m_systemVersion;
};

#endif  // BASEINFO_H
