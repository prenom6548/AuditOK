#ifndef LEVELSCROLLDISKVIEW_H
#define LEVELSCROLLDISKVIEW_H

#include <QObject>
#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QEvent>
#include <QMouseEvent>
#include <QProcess>
#include "diskinfoview.h"
#include "partman/device.h"
#include "QButtonGroup"

using namespace KInstaller;
using namespace Partman;

class LevelScrollDiskView : public QWidget
{
    Q_OBJECT
public:
    explicit LevelScrollDiskView(QWidget *parent = nullptr);
    void initUI();
    void initAllConnect();
    void addItemsToList(DeviceList devlist);
//    void addDot();
    void showBtn();

    void setListwidgetSize(QSize size);
    QSize getListwidgetSize();
    void addStyleSheet();
    void setListItem(int id);

    QListWidget* getListWidget();
    QString getDiskFormat(QString diskPath);

    void getDiskSizeLimits(QString devpath, qint64& sizeDataCurrent, qint64& sizeSysCurrent, qint64& nDataMiniSize, qint64& nDataMaxSize);
    bool lsLVMDisk(QString devPath);

private:
//    bool eventFilter(QObject *watched, QEvent *event);
//    void paintEvent();
    int checkDisk();
signals:
    void signalWidgetSelected(QString dev);//实参不为空表示选中实参硬盘作为系统安装盘；实参为空表示没有选中任何盘作为系统安装盘
    void signalDataDiskSelected(bool value);//实参true表示选中某个硬盘作为数据盘;false表示没有任何硬盘被选中作为数据盘
    void signalCheckSavehistory(QString devpath);

public slots:
//    void clickLeftBtn();
//    void clickRightBtn();
//    void showWidgetCheckdIteed(QString dev);
    void showWidgetChecked(QString devpath);
    void selectView(int k);

    void checkDiskLeft();
    void checkDiskRight();
public:
    QListWidget *listWidget;//显示硬盘的区域
    QList<DiskInfoView*> m_disklist;//全部硬盘图形及数据列表
    QButtonGroup m_Group;
    DiskInfoView* disk;
    bool blViewPic;
    int numDisk;
    QString m_curDevPath;
    QPushButton *leftBtn, *rightBtn;
    QGridLayout *dotlayout;
    QList<QLabel*> dotlist;
    int movesize;

    int DataDeviceIndex=-1;//表示被勾选上作为数据盘的硬盘对应队列m_disklist中的下标
    bool m_SSDBoolValue = false;
    bool m_HDDBoolValue = false;
    int  m_SSDIndex = -1;
    int  m_HDDIndex = -1;
    QHBoxLayout *layout;
    QHBoxLayout* hlistlayout;

    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event);


};

#endif // LEVELSCROLLDISKVIEW_H
