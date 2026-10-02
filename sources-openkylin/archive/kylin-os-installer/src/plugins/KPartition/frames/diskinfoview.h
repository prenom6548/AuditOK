#ifndef DISKINFOVIEW_H
#define DISKINFOVIEW_H

#include <QWidget>
#include <QFrame>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "disktype_name.h"
#include "qcheckbox.h"
#include "pushbuttonicon.h"
#include "setPartitionsSize.h"
#include "../PluginService/ksystemsetting_unit.h"
#include <QPushButton>
#include <QStackedWidget>

using namespace KInstaller;

class DiskInfoView : public QWidget
{
    Q_OBJECT
public:
    explicit DiskInfoView(QWidget *parent = nullptr);
    void addStyleSheet();
    void initAllConnect();
    void translateStr();
    QString devPath;                      //本设备地址，如/dev/sda
    void setDevPath(QString dev) {devPath = dev;}
    QString getDevPath() {return devPath;}
    QSize getFramesize();
    void setWidgetDisabled();
    void setProgressStyle();
    void setPartiSizeBtVisible(bool bStatus);
    void setSaveHistoryStatus(bool bStatus);
    void moveToLocal(void);

    void setDiskType(QString type) {m_strDiskType = type; diskType->setText(m_strDiskType);}
    void setDiskName(QString name) {m_strDiskNameReal = name; diskName->setText(m_strDiskNameReal);}
    void setDiskSize(QString size) {m_strDiskSize = size; diskSize->setText(m_strDiskSize);}
    void setDiskPath(QString path) {m_strDiskpath = path; diskPath->setText(m_strDiskpath);}
    void getDiskSizeInfo(qint64 nTatolSize, qint64 nDataSize, qint64 nDataMiniSize, qint64 nDataMaxSize){m_sizeTotal=nTatolSize;m_sizeDataCurrent=nDataSize;m_nDataMiniSize=nDataMiniSize;m_nDataMaxSize=nDataMaxSize;translateStr();}

    QString getDiskType() {m_strDiskType = diskType->text(); return m_strDiskType; }
    QString getDiskName() {m_strDiskNameReal = diskName->text(); return m_strDiskNameReal;}
    QString getDiskSize() {m_strDiskSize = diskSize->text(); return m_strDiskSize; }
    QString getDiskPath() {m_strDiskpath = diskPath->text(); return m_strDiskpath;}

    void setProgressValue(double value) {progressBar->setValue(value);}

private:
    void changeEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *);


protected:
    bool eventFilter(QObject *object, QEvent *event);

signals:
    void signalClicked(QString dev);
    void signalClickView(int k);       //dataSel被点击了
//    void signalDateChangeclicked();

public slots:
    void slotFrameClicked();
    void clickDataSel();
    void setSizeButtClicked();

public:
    QWidget *widget;                    //总界面
    QWidget *layoutWidget;              //子控件布局界面

    QVBoxLayout *verticalLayout;
    QLabel *diskType;
    QWidget *m_diskPathandNameWidget;
    QLabel *diskPath;                   //设备挂载点
    QLabel *diskName;                   //设备名称
//    QLabel* m_diskNameTip;            //设备名称的悬浮提示，当设备名称过长时显示
    QString m_strDiskNameReal;          //设备名称,全部信息
    QString m_strdiskNameShow;          //设备名称,展示信息，当信息过长时会有省略号
    QProgressBar *progressBar;
    QWidget *m_SizeLineWidget;
    QLabel *diskSize;                   //显示磁盘信息，如：已用容量/总容量，加密磁盘，LVM/总容量
    QLabel *hardDiskPng;
    QLabel *checkpic;                   //勾号图形。显示此图形表示本硬盘被选中作为系统安装盘
    bool m_checkable;                   //值true表示本硬盘容量不小于50G
    int mIndex;                         //本设备对应上层数据列表中的下标，从0开始
    QCheckBox *dataSel;                 //“设为数据盘”勾选框
    int m_isEncrypted;                  //本设备中含有被加密分区时为1,含有lvm分区时为2，否则为0

    QStackedWidget*  m_dataSelandSet;
    QWidget *m_setSizeWidget;
    QHBoxLayout *m_horizLayout;
    QLabel *m_sysParSize;               //系统分区当前大小展示
//    QLabel* m_sysTip;                   //系统分区大小的悬浮提示，当此字符串过长时显示
    QString m_strSysFull;               //系统分区大小的展示全部内容
    QString m_strSysShow;               //系统分区大小的展示内容，可能最后带省略号
    QLabel *m_dataParSize;              //数据分区当前大小展示
//    QLabel* m_dataTip;                  //数据分区大小的悬浮提示，当此字符串过长时显示
    QString m_strdateFull;              //数据分区大小的展示全部内容
    QString m_strDataShow;              //数据分区大小的展示内容，可能最后带省略号
    QLabel *m_space;
    QWidget *m_DateFocseWidget;
    QWidget *m_DateBackgroundWidget;

//    QWidget* m_pBottWiget = nullptr;
    QLabel *m_setPartitSize = nullptr;  //调节分区大小按钮
//    mylabelicon * m_licon = nullptr;
//    QPushButton* m_setPartitSizeBTN;
//    SetPartitionsSize *m_setPartitSizeWidget;
    qint64 m_sizeTotal = 0;             //此设备总大小。单位字节
    qint64 m_sizeDataCurrent = 0;       //此设备最初数据分区大小。单位字节
    qint64 m_sizeDataSetted = 0;        //用户当前设置的数据分区大小。单位字节
    qint64 m_nDataMiniSize = 0;         //单位字节
    qint64 m_nDataMaxSize = 0;          //单位字节
    double m_sizeSysShow = 0.0;         //系统分区当前大小，单位GB
    double m_sizeDataShow = 0.0;        //数据分区当前大小，单位GB
    bool m_setSize = false;             //本设备是否设置过数据分区大小

    bool m_saveHistory = false;         //是否勾选了“保留用户及其数据”
    bool m_history = false;             //本设备中是否已经安装了符合要求的麒麟操作系统

    QString m_strDiskType, m_strDiskSize, m_strDiskpath;
};

#endif // DISKINFOVIEW_H
