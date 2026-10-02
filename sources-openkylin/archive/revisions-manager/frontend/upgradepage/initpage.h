#ifndef INITPAGE_H
#define INITPAGE_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QDBusInterface>
#include <QDBusReply>
#include "dbusconnect.h"
class initpage: public QWidget
{
    Q_OBJECT
public:
    explicit initpage(QWidget *parent = nullptr);
    void initui();
    void dbusconnection();

    QLabel* iconlabel;
    QPushButton* UpgradeBtn;
    QPushButton* RestoreBtn;
//    QPushButton* closeBtn;
    QVBoxLayout* mainlayout;
//    QHBoxLayout* iconlayout;
public slots:
    void Upgrade(bool);
    void Restore(bool);
signals:
    void Upgradesignal();
    void Restoresignal();
    void closesignal();
};

#endif // INITPAGE_H
