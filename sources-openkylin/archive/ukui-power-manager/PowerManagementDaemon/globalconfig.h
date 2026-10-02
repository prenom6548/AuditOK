#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include <QObject>
#include <QMetaType>
#include <QList>
#include <QDBusInterface>

class GlobalConfig : public QObject
{
    Q_OBJECT
public:
    explicit GlobalConfig(QObject *parent = nullptr);
    ~GlobalConfig();

    void initGlobalConfig();

    bool writeGlobalParam(const QString & key, const QVariant &value);

    bool readAllGlobalParam(QVariantMap &oMap);
Q_SIGNALS:
    void  userActiveSignal(bool);

private Q_SLOTS:
    void userActiveSlot(bool);

private:
    bool m_isInit = false;
    QDBusInterface *m_globalBus  = nullptr;
};

#endif // GLOBALCONFIG_H
