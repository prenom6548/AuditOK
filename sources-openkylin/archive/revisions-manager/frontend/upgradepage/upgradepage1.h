#ifndef UPGRADEPAGE1_H
#define UPGRADEPAGE1_H

#include <QWidget>
#include <QLabel>
#include <QIcon>
#include <QPushButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QDebug>
#include <QCheckBox>
#include <QFileDialog>
#include <kborderlessbutton.h>
#include <ktoolbutton.h>
#include <QDBusInterface>
#include <QDBusReply>
#include "dbusconnect.h"
using namespace kdk;
class upgradepage1: public QWidget
{
    Q_OBJECT
public:
    explicit upgradepage1(QWidget *parent = nullptr);

    void initui();
    void dbusconnection();


    QWidget* midwidget;
    QVBoxLayout *midlayout;
    QHBoxLayout *bigmidlayout;

    QLabel* iconlabel;
    QIcon* icon;
    QLabel* version;
    QLabel* tiplabel;
//    QLabel* tiplabel1;
    QLineEdit* fileline;
    KToolButton* filebtn;
    QPushButton *importactivationfilebutton;
    QPushButton* upgradebtn;
    QPushButton* backbtn;
//    QSpacerItem* spacer;
    KBorderlessButton* attentionbtn;
    QPushButton* exitbtn;
    QCheckBox *switchAppMoveCheckbox;
//    QLabel *switchAppMovelabel;
    KBorderlessButton *advancedoptionsbutton;

    QHBoxLayout *exitbuttonlayout;
    QHBoxLayout* iconlayout;
    QHBoxLayout* versionlayout;
    QHBoxLayout* tiplayout;
    QHBoxLayout* filelayout;
    QHBoxLayout* upgradelayout;
    QHBoxLayout* bottomlayout;
    QHBoxLayout *switchapplayout;
    QVBoxLayout* mainlayout;

    dbusconnect *dbus;

    QString s;

private slots:
    void showfile(bool);

public slots:
    void isocontentedited(const QString &text);
    void emitsignal(bool);
    void emitbacksignal(bool);
    void showattention(bool);
    void exitapp(bool);
    void switchappmovecheck(int);
    void showadvancedoptions(bool);

signals:
    void exitappsignal();
    void nextpage();
    void backpage();
    void showattentionsignal();
    void showadvancedoptionspage();
};

#endif // UPGRADEPAGE1_H
