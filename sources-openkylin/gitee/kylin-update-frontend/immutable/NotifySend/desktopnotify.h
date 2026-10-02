#ifndef DESKTOPNOTIFY_H
#define DESKTOPNOTIFY_H

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
class DesktopNotify : public QWidget
{
    Q_OBJECT
public:
    explicit DesktopNotify(QWidget *parent = nullptr);
    ~DesktopNotify();
    int ID=0;
    QTimer *timer;
    QString getVersion();
    QString version_t = "";
    QString version;
signals:

public Q_SLOTS:
    void RebootNotify();

public slots:
    void ButtonAction_2309(uint id, QString key);
    void timenotification_2309();
};

#endif // DESKTOPNOTIFY_H
