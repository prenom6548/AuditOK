#ifndef ADVANCEDOPTIONS_H
#define ADVANCEDOPTIONS_H
#include <QWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLabel>
#include <QScrollArea>
#include <QLineEdit>
#include <QToolButton>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QFileDialog>
#include <QList>
#include <QCheckBox>
#include"dbusconnect.h"
#include <QApplication>
#include <kborderlessbutton.h>
using namespace kdk;
class advancedoptions: public QWidget
{
    Q_OBJECT
public:
    explicit advancedoptions(QWidget *parent = nullptr);
    ~advancedoptions();
    QLabel *advancedoptiontitle;

    QLabel *KeepedThirdPartPackagesTitle;
    QListWidget *KeepedThirdPartPackages;
//    QPushButton *addthridpartpackagebutton;
//    QPushButton *removethridpartpackagebutton;
    KBorderlessButton *SelectAll;
    KBorderlessButton *ReverseAll;
    QPushButton *okbutton;
    QPushButton *cancelbutton;

    QVBoxLayout *mainlayout;
    QHBoxLayout *advancedoptionlayout;
    QHBoxLayout *activationfilelayout;
    QVBoxLayout *thirdpartlayout;

    QHBoxLayout *SelectAllReverseAllLayout;
    QHBoxLayout *KeepedThirdPartPackagestitlelayout;
    QHBoxLayout *KeepedThirdPartPackageslayout;
    QHBoxLayout *addremoveKeepedThirdPartPackagesbuttonlayout;
    QHBoxLayout *SelectAllLayout;
    QHBoxLayout *ReverseAllLayout;
    QHBoxLayout *okcancelbuttonlayout;

    dbusconnect *dbus;
    QStringList selecteditemsname;

    void initui();
    void initconnections();
    void initapplist();
    void setupselecteditems();
public slots:
    void additem();
    void removeitem();
    void edititem(QListWidgetItem *);
    void ok();
    void cancel();
    void selectall();
    void reverseall();
signals:
    void exitadvancedoptionsettings();
};


#endif // ADVANCEDOPTIONS_H
