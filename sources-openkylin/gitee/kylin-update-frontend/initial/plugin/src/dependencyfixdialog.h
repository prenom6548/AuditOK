#ifndef DEPENDENCYFIXDIALOG_H
#define DEPENDENCYFIXDIALOG_H
/*该文件是在需要修复依赖时进行弹窗*/
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QString>
#include <QCloseEvent>

#include <kdialog.h>
#include <kborderlessbutton.h>
using namespace kdk;

#include <ukcc/widgets/fixlabel.h>

#include "updatedbus.h"

enum{
    updateAll=1,
    updatePart,
    updateSystem,
};

class dependencyfixdialog : public QDialog
{
    Q_OBJECT
public:
    dependencyfixdialog(int mode);

    KBorderlessButton *details;
    QPushButton *yesbtn;
    QPushButton *nobtn;

    FixLabel *attention;
    QLabel *warning;
    void initui();
    void dbusconnect();
    int updatedeletepkglist(QStringList pkgname,QStringList description,QStringList deletereason);

    int versionlabelflag=0;

    int updatemode=0;
    QStringList pkgname;
    QStringList description;
    QStringList deletereason;

public slots:
    void fixdependency();
    void nofixdependency();
    void showdetails();

signals:
    void updatedependsolvecancelsignal(void);
    void updatedependsolveacceptsignal(void);
    void updatealldependsolveacceptsignal(bool);
    void disupdatedependsolveacceptsignal();
    void updatedependshowdetailssignal(QStringList pkgname,QStringList description,QStringList deletereason,int updatemode);
    void closedpendencyfixdialog();

protected:
    void closeEvent(QCloseEvent *e);
};

#endif // DEPENDENCYFIXDIALOG_H
