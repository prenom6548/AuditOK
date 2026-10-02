#ifndef CUSTOMPARTITIONFRAME_H
#define CUSTOMPARTITIONFRAME_H

#include <QObject>
#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QScrollArea>
#include "frames/progresslabel.h"

#include "partman/device.h"
#include "partman/partition.h"
#include "frames/tablewidgetview.h"
#include "createpartitionframe.h"
#include "./delegate/custom_partition_delegate.h"
#include "frames/diskpartitioncolorprogress.h"
#include "modifypartitionframe.h"
namespace KInstaller{
class CustomPartitionFrame : public QWidget
{
    Q_OBJECT
public:
    explicit CustomPartitionFrame(CustomPartitiondelegate* customDelegate, QWidget *parent = nullptr);
    ~CustomPartitionFrame();

    void initUI();
    void initAllConnect();
    void translateStr();
    void addStyleSheet();
    void addPartProgress();

    void addTableWidget();
    void newTableViews(DeviceList devs);
    void newLvmTableViews(DeviceList devs);
//    void initLayout();
    void revertPartition();
    void updateScrollAreaWidget();


signals:
    void signalNextBtn(bool flag);
    void signalCreatePartTable(Device::Ptr& dev);
    void signalFormatDisk(Device::Ptr dev);
    void signalFinishedLoadDisk();
    void enterpressed();
    void backspacepressed();
    void createLvm();
    void updateLvm();
    void reSetFocusPolicy();
    void modifyPartFinished();  //修改分区子窗口结束信号
private:
    void changeEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

public slots:
    void slotCreatePartition(Device::Ptr dev, Partition::Ptr part);//增加一个分区
    void slotDeletePartition(Device::Ptr dev, Partition::Ptr part);//删除一个分区
    void slotModifyPartition(Device::Ptr dev, Partition::Ptr part);//修改一个分区
    void setLvmPartition(Device::Ptr dev, Partition::Ptr part);//修改一个分区
    void slotRestorePartition();
    void slotSelectBootCombox(QString bootstr);
    void repaintDevice();
    void lvmPartition();
    void createLvmPartition();
    void updateTableView(TableWidgetView* tablewidget);

public:
    QList<TableWidgetView*> m_tableViews;               //设备框列表
    QGridLayout* gridLayout;                            //自定义安装的总界面
    DiskPartitionColorProgress* m_colorProgressFrame;   //分区大小比例条，位于总界面最上面
    //新建分区表New partition table,还原revert
    QPushButton* m_revertBtn;
    QPushButton* m_lvmBtn;
    QLabel* m_mainTitle;
    QScrollArea* pScroll;             //包含分区列表的滑动框，其中包含了“创建分区表”按钮，以及分区列表等。

    DeviceList devlist;               //硬盘列表
    CustomPartitiondelegate* m_delegate;
    CreatePartitionFrame* m_createPartFrame;  //新建分区子窗口
    ModifyPartitionFrame* m_modifyPartFrame;  //修改分区子窗口
    QLabel* m_bootlabel;
    QComboBox* m_bootCombox;
    QLabel* m_ToolTip;                 //当前硬盘名全称的悬浮提示，当硬盘名称过长时显示
    QString m_bootstr;
    Device::Ptr m_dev;

    QWidget* tableViewWidget ;         //分区列表总界面，包含了pScroll。位于总界面中间
    QWidget* psWidget;
    QVBoxLayout* vtlayout;              //分区列表界面
    int layout1Width;

    QWidget *m_pFrame;
    int m_SCreenWidth;
    int m_SCreenHeight;

    // QWidget interface
protected:
    void showEvent(QShowEvent *event);
};
}
#endif // CUSTOMPARTITIONFRAME_H
