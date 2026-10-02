#ifndef REBOOTINSTALLNOTIFY_H
#define REBOOTINSTALLNOTIFY_H

#include <QObject>
#include <QWidget>
#include <QDBusMessage>
#include <QtDBus/QtDBus>
#include <QTime>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QObject>
#include <QTranslator>
#include <QDBusInterface>
#include <QTimer>
#include <QMetaType>
class RebootInstallNotify : public QWidget
{
    Q_OBJECT
public:
    explicit RebootInstallNotify(QWidget *parent = nullptr);
    QTimer *timer;
    QTimer *timer1;
    int ID = 0;
public slots:
    void timenotification_2107();
    void hourdebug();
    void ButtonAction(int id ,QString key);
signals:

};

#endif // REBOOTINSTALLNOTIFY_H
