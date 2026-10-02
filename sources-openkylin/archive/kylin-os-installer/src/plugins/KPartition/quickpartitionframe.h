#ifndef QUIKPARTITIONFRAME_H
#define QUIKPARTITIONFRAME_H
#include <parted/parted.h>
#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QStackedLayout>
#include <QWidgetList>
#include "frames/diskinfoview.h"
#include "frames/levelscrolldiskview.h"
#include "partman/partition_server.h"
#include "createpartitionframe.h"
#include "custompartitionframe.h"
#include "delegate/full_partition_delegate.h"
#include "./frames/encryptsetframe.h"

namespace KInstaller{
enum class PARTTYPEBTN {
    quikInstall,
    coexistInstall,
    customInstall
};
class FullPartitionFrame : public QWidget
{
    Q_OBJECT
public:
    explicit FullPartitionFrame(FullPartitionDelegate* quickDelegate, QWidget *parent = nullptr);

public:
    void initUI();
    void initAllConnect();
    void translateStr();
    void addStyleSheet();

//    QWidgetList getChildWidgets();
    void showListDisk();
    void showTableDisk();
//    void addItemsToList();


signals:
    void signalSeclectedListItem(bool flag);
    void signalFinishedLoadDisk(int DiskNum);//
    void enterpressed();
    void backspacepressed();
    void leftCliked();
    void rightClicked();
    void reSetFocusPolicy();
private:
    void changeEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event);
public slots:

//    void activatedItem(QListWidgetItem *);
//    void ChangedItem(QListWidgetItem*,QListWidgetItem*);
    void currentDiskID(QString devpath);
    void repaintDevices(DeviceList devs);
    void setEncryptyToDisk();
    void setLvmToDisk();
    void DataDiskCheck(bool value);
    void defaultOption(QString objectName);
    void slotCheckSavehistory(QString devpath);

public:

    LevelScrollDiskView *slistDisk;

    PARTTYPEBTN partType;
    QLabel* promptLabel;

    QStackedLayout* m_midStackLayout;
    FullPartitionDelegate* m_quickDelegate;
    int m_devCount;
    QString m_devPath;
    QLabel* m_tip;

    DeviceList devlist;
    QTimer* timer;
    QLabel* m_loadpic;
    bool firstbackShow = true;
    int var;
    int num=1;
    //加密
    QCheckBox* m_encryptBox;
    //lvm
    QCheckBox* m_lvm;
    //出厂备份
    QCheckBox* m_firstback;
    //保留历史数据
    QCheckBox* m_savehistory;

    MessageBox *message;
    QString historyDev;
    QString isGhost;
    bool  m_display;                            //false值表示还没有执行过函数defaultOption()；true值表示执行过。

    EncryptSetFrame *m_EncryptSetframe=NULL;


    // QWidget interface
protected:
    void showEvent(QShowEvent *event);
};
}
#endif // PARTITIONFRAME_H
