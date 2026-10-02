#ifndef CONFIG_H
#define CONFIG_H

#include <QFile>
#include <QMap>
#include <QObject>
#include "common.h"

class QGSettings;

class Config : public QObject
{
    Q_OBJECT
public:
    explicit Config(QObject *parent = Q_NULLPTR);
    ~Config();

    void setGsettings(const QString &key, const QVariant &value);
    QVariant getGsettings(const QString &key);
private:
    bool isNeedConfig();
    void initConfig();
    QString getProduct();
    int getManufacturer();
    int vendorTypeMatch(const QString &type);
    void vendorSpecialTreatment();


    QPointer<QGSettings> m_gsettings;
    bool m_isSpecial;
    QMap<QString, QVariant> m_specialData;


    enum vendor
    {
        Unknown = 0,
        Greatwall,
    };
};

#endif // CONFIG_H
