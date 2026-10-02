#ifndef RESTOREPAGE1_H
#define RESTOREPAGE1_H

#include <QObject>
#include <QWidget>
#include <QIcon>
#include <QPushButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QDebug>
#include <QLabel>

#include <QFileDialog>

#include "dbusconnect.h"

class restorepage1 : public QWidget
{
    Q_OBJECT
public:
    explicit restorepage1(QWidget *parent = nullptr);

    void initui();
    void dbusconnection();

    QLabel* iconlabel;
    QIcon* icon;
    QLabel* version;
    QLabel* tiplabel;
    QPushButton* restorebtn;
    QPushButton* attentionbtn;
    QPushButton* exitbtn;

    QHBoxLayout* iconlayout;
    QHBoxLayout* versionlayout;
    QHBoxLayout* tiplayout;
//    QHBoxLayout* filelayout;
    QHBoxLayout* upgradelayout;
    QHBoxLayout* bottomlayout;

    QVBoxLayout* mainlayout;

    dbusconnect *dbus;

    int over;

public slots:
    void emitsignal(bool);
    void showattention(bool);
    void exitapp(bool);

signals:
    void exitappsignal();
    void nextpage();
    void showattentionsignal();
};

#endif // RESTOREPAGE1_H
