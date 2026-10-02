#ifndef IMPORTACTIVATIONFILESPAGE_H
#define IMPORTACTIVATIONFILESPAGE_H
#include<QWidget>
#include<QLabel>
#include<QPushButton>
//#include<QFrame>
#include<QSpacerItem>
#include<QHBoxLayout>
#include<QVBoxLayout>
#include<QIcon>
#include<QFileInfo>
#include"dbusconnect.h"
class importactivationfiles: public QWidget
{
    Q_OBJECT
public:
    explicit importactivationfiles(QWidget *parent = nullptr);
    ~importactivationfiles();
    void initui();
    void initconnections();
    bool checklicensefileformat(QString);
    QString kyinfo;
    QString license;
    QLabel *systemactivationtitle;
    QLabel *activationtiptitle;
    QPushButton *importactivationfilesbutton;

//    QFrame *kyifoframe;
//    QIcon *kyinfoicon;
    QLabel *kyinfolabel;
//    QIcon *kyinfodeleteicon;
    QPushButton *kyinfodeletebutton;

//    QFrame *licenseframe;
//    QIcon *licenseicon;
    QLabel *licenselabel;
//    QIcon *licensedeleteicon;
    QPushButton *licensedeletebutton;
//    QSpacerItem *spacefiller1;

    QPushButton *upgradebutton;
    QPushButton *laststepbutton;

    QVBoxLayout *mainlayout;
    QHBoxLayout *systemactivationtitlelayout;
    QHBoxLayout *activationtiplayout;
    QHBoxLayout *importactivationfilesbuttonlayout;
    QHBoxLayout *kyinfolayout;
    QHBoxLayout *licenselayout;
    QHBoxLayout *upgradebuttonlayout;
    QHBoxLayout *laststepbuttonlayout;
    dbusconnect *dbus;

public slots:
    void chooseactivationfiles();
    void upgrade();
    void laststep();
    void deletekyinfofile();
    void deletelicensefile();

signals:
    void nextpage();
    void lastpage();
};

#endif // IMPORTACTIVATIONFILESPAGE_H
