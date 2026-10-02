#ifndef SETPARTITIONSSIZE_H
#define SETPARTITIONSSIZE_H

#include <QObject>
#include <QLabel>
#include <QPushButton>
#include <QDialog>
#include <QLineEdit>
#include <QWidget>
#include <QAction>
#include "../PluginService/ui_unit/arrowtooltip.h"
#include "../../KUserRegister/clineeditlabel.h"
#include "../../KUserRegister/checkedlineedit.h"
#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>
#include <QSlider>
#include <QProgressBar>

using namespace KInstaller;
namespace KServer {

class SetPartitionsSize : public QDialog
{
    Q_OBJECT
public:
    explicit SetPartitionsSize(QWidget *parent = nullptr);
    void initUI();                                     // 加载界面ui
    void initCloseButton();                            // 初始化关闭按钮界面
    void initSysandDataQScWidget();                    // 初始化比例条界面
    void initChangeSizeWidget();                       // 初始化分区大小调节界面
    void initButtonWidget();                           // 初始化取消和确认按钮界面

    void getDiskInfo(QString strDiskName, QString strDiskPath, qint64 nTatolSize, qint64 nDataSizeOri, qint64 nDataSizeSetted, qint64 nDataMiniSize, qint64 nDataMaxSize);//全盘安装界面初始化后由调用者立刻单独调用（即修改函数内代码void LevelScrollDiskView::addItemsToList(DeviceList devlist)内对disk变量的赋值。void FullPartitionFrame::repaintDevices(DeviceList devs)）

//    void initPasswdWidget();                            // 初始化输入密码界面
//    void initConmfirmPasswdWidget();                    // 初始化确认输入密码界面
//    void initCheckpwdSafeWidget();                      // 初始化提示保管密码界面

//    QString SetFormatBody(QString text, QLabel *label);    // 设置字体...
//    int GetFormatWidth(QLabel *label);              // 获取当前语言下“密码”和“确认密码”栏的最大宽度
    void initAllConnect();                              // 信号和槽链接
    void addStyleSheet();                               // 设置样式
    void translateStr();                                // 更新国际化翻译，并根据m_sizeSetting值来更新各个控件显示的值
    void checkInput(qint64 nSizeNow);
    void checkAcceptAble();
//    void setMessageInfo(QString info);
//    void setCancleHidden();
////    void markLineEditCorrect(LabelState state, bool flag);
//    void addLineditAction();
//    int execid;
signals:
    void signalCancel();
    void signalOK();

private:
//    void changeEvent(QEvent *event) override;       //改变语言事件
    void resizeEvent(QResizeEvent* event);
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
    void keyPressEvent(QKeyEvent *event);

public:
    QString m_strDeviceName;
    QString m_strDiskPath;
    qint64 m_sizeTotal = 0;//此设备总大小。单位字节
    qint64 m_sizeDataCurrent = 0;//最初数据分区大小。单位字节
    qint64 m_sizeSystemCurrent = 0;//当前全部系统分区的大小。单位字节
    qint64 m_sizeSetting = 0;//用户当前设置的数据分区大小。单位字节
    qint64 m_nDataMiniSize = 0;//单位字节
    qint64 m_nDataMaxSize = 0;//单位字节
    bool m_acceptAble = false;//用户输入大小是否合理

    // 主窗口垂直布局
    QVBoxLayout* m_pMainVLayout = nullptr;

    // 关闭窗口按钮窗口
    QWidget *m_pCloseButtonWidget = nullptr;
    QHBoxLayout* m_pCloseButtonHLayout = nullptr;
    QPushButton *m_pCloseButton = nullptr;
    ArrowWidget *m_tTip = nullptr;//关闭按钮的悬浮提示

    //分区大小比例条
    QWidget *m_pTatolSizeWidget = nullptr;
    QVBoxLayout* m_pTatolSizeVLayout = nullptr;
    QLabel* m_pTatolSizeLabel = nullptr;//硬盘名称和大小
    QProgressBar* m_pSysandDataSizeQB = nullptr; //比例条
    QLabel* m_pSysSizePic = nullptr;
    QLabel* m_pSysSizeLabel = nullptr;
    QLabel* m_pDataSizePic = nullptr;
    QLabel* m_pDataSizeLabel = nullptr;

    //分割线
    QFrame* m_pDeliveLine = nullptr;

    //分区大小调节区
    QWidget *m_pChangeSizeWidget = nullptr;
    QGridLayout* m_pChangeSizeGLayout = nullptr;
    QLabel* m_pChangeSizeLabel = nullptr;
    QSlider *m_pSizeQSlider = nullptr;//分区大小调节滑块
    QLineEdit *m_pSize = nullptr;//分区大小输入框
    QLabel* m_pUnitLabel = nullptr;
    QLabel* m_pDataSizeRangTit = nullptr;

    // 取消和确认按钮界面
    QWidget *m_pButtonWidget = nullptr;
    QHBoxLayout *m_pButtonHBoxLayout = nullptr;
    QPushButton *m_pCancelButton = nullptr;
    QPushButton *m_pOkButton = nullptr;

    bool bInputBegin = false;

signals:
//     void signalClickBtn(int execid);

public slots:
     void SetSysSize(int nValue);
     void SaveSysSize(QString strSize);
//     void FinishInputSize();

     // QWidget interface
protected:
     void showEvent(QShowEvent *event);
};

}
#endif // SETPARTITIONSSIZE_H
