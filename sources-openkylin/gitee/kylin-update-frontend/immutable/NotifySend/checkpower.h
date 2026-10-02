#ifndef CHECKPOWER_H
#define CHECKPOWER_H

#include <QObject>
#include <QWidget>
#include <QDBusMessage>
#include <QtDBus/QtDBus>
#include <QSettings>
#include <QDBusInterface>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QObject>
#include <QTranslator>
#include <QMetaType>
class CheckPower: public QWidget
{
    Q_OBJECT
public:
    CheckPower();
};

#endif // CHECKPOWER_H
