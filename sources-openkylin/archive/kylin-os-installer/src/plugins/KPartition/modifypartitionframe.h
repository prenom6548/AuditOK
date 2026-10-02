#ifndef MODIFYPARTITIONFRAME_H
#define MODIFYPARTITIONFRAME_H

#include <QObject>
#include <QWidget>
#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QRadioButton>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QButtonGroup>
#include <QCheckBox>
#include <QKeyEvent>

#include "partman/device.h"
#include "partman/partition.h"
#include "frames/lineeditaddreduce.h"
#include "./delegate/custom_partition_delegate.h"
#include "../PluginService/ui_unit/messagebox.h"
#include "../PluginService/ui_unit/arrowtooltip.h"


namespace KInstaller {
using namespace Partman;

class ModifyPartitionFrame : public QDialog
{
    Q_OBJECT
public:
    explicit ModifyPartitionFrame(CustomPartitiondelegate* delegate, QWidget *parent = nullptr);
    ~ModifyPartitionFrame();
    void setModifyPartition( Partition::Ptr partition,int SCreenWidth,int SCreenHeight);
    void initUI();
    void initAllConnect();
    void translateStr();
    void addStyleSheet();
    void initBackground();

private:
    void changeEvent(QEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
 signals:
        void finished();
	void createLvm();
        void signalCloseBtn();
public slots:
    void changeFileFormat(int index);
    void changeMountFile(const QString &text);

    void slotOKBtn();

public:
    QLabel *tipLabel, *m_mainTitle, *fsLabel, *mountLabel;
    QComboBox *fileFormat, *mountFile;                          //分区类型下拉框，挂载点下拉框
    QCheckBox* checkFormat;                                     //“格式化此分区”勾选框
    QStringList filesystemList, mountedList;                    //分区类型下拉框中的内容列表，挂载点下拉框中的内容列表
    QString fileFormatStr, mountFileStr;                        //初始化本类对象时使用的文件系统类型，挂载点
    QString isPvFormat; //用于记录之前分区是否为pv分区
    QString realFileFormat;
    QPushButton* m_close;                                       //右上方的关闭按钮
    QPushButton* m_OKBtn;                                       //最下方的确定按钮
    CustomPartitiondelegate* m_delegate = nullptr;
    Device::Ptr m_dev;
    Partition::Ptr m_partition;                                  //指向当前分区对象
    QString m_filesystem;                                        //存放 分区类型下拉框 中当前选中的内容
    bool m_blCreatorPartition;
    MessageBox *message;
    ArrowWidget *closeTip;
    DeviceList *devlst;                                          //硬盘列表
    int m_SCreenWidth;
    int m_SCreenHeight;
    int m_focusgetnum=0;
    bool searchEfi=true;
    // QWidget interface
protected:
    void showEvent(QShowEvent *event);
protected:
    void keyPressEvent(QKeyEvent *event);

};
}
#endif // MODIFYPARTITIONFRAME_H
