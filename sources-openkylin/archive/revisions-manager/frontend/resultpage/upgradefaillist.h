#ifndef UPGRADEFAILLIST_H
#define UPGRADEFAILLIST_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QList>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>

#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

class upgradefaillist : public QWidget
{
    Q_OBJECT
public:
    explicit upgradefaillist(QWidget *parent = nullptr);

    QLabel* toplabel;
    QLabel* toplabel1;
    QListWidget* deblist;
    QPushButton* confirm;

    QWidget* topwidget;
    QHBoxLayout* toplayout;

    QVBoxLayout* mainlayout;

    void initui();
    void dbusconnection();
    void backtopage();
    QStringList getdeblist();
signals:
    void backtopagesignal();

};

#endif // UPGRADEFAILLIST_H
