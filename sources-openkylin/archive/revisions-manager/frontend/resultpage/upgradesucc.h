#ifndef UPGRADESUCC_H
#define UPGRADESUCC_H

#include <QObject>
#include <QWidget>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QIcon>
#include <QTextEdit>
#include"dbusconnect.h"

class UpgradeSucc : public QWidget
{
    Q_OBJECT
public:
    explicit UpgradeSucc(QWidget *parent = nullptr);

    QLabel* iconlabel;
    QIcon* icon;
    QLabel* warning;
    QLabel* detail;
    QLabel* tips;
    QTextEdit* content;
    QPushButton* btn;

    QVBoxLayout* mainlayout;
    QVBoxLayout* iconlayout;
    QVBoxLayout* warninglayout;
    QVBoxLayout* titlelayout;
    QVBoxLayout* detaillayout;
    QVBoxLayout* btnlayoutlayout;

    QWidget* btnwidget;
    dbusconnect *dbus;
    void initui();
    void dbusconnection();

public slots:
    void exitapp();

signals:
    void exitappsignal();

};

#endif // UPGRADESUCC_H
