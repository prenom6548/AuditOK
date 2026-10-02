#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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
#include <QTimer>
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    int ID=0;
    QTimer *timer;
    QString getVersion();
    QString version_t = "";
    QString version;
public Q_SLOTS:
    void RebootNotify(bool success);
    void RebootResult(bool success,QString error_code,QString reason);

public slots:
    void ButtonAction_2309(uint id, QString key);
    void timenotification_2309();
};
#endif // MAINWINDOW_H
