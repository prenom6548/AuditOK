/*Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "powerprovider.h"
#include "lockchecker.h"
#include "mypushbutton.h"
#include "commonpushbutton.h"
#include "systemupgradecheck.h"

#include <QMainWindow>
#include <QTimer>
#include <QProcess>
#include <QPixmap>
#include <QHash>
#include <QGSettings/QGSettings>
#include <QAbstractNativeEventFilter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSettings>
#include <QScrollArea>
#include <QKeyEvent>

class XEventMonitor;
class QListView;
class QStandardItemModel;
class MyListView;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    /**
     * @brief 响应按钮事件
     * @param test2 按钮名字
     * @param i 按钮序号
     */
    void doEvent(QString test2,int i);

    /**
     * @brief 响应鼠标事件后改变按钮状态
     * @param widget 指定按钮对象
     * @param widget 鼠标事件
     */
    void changePoint(QWidget *widget ,QEvent *event);

    /**
     * @brief 调整界面显示的控件
     */
    void judgeboxShow();

    /**
     * @brief 消息过滤
     */
    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;

    /**
     * @brief 初始化系统监视器按钮
     */
    void initialSystemMonitor();

    /**
     * @brief 初始化按钮
     */
    void initialBtn();

    /**
     * @brief 初始化提示内容的控件
     */
    void initialJudgeWidget();

    /**
     * @brief 初始化时间控件
     */
    void initialDateTimeWidget();

    /**
    * @brief 更新时间控件
    */
    void updateDateTimerWidget();

    /**
     * @brief 可以通过配置文件来控制按钮是否显示
     */
    void initialBtnCfg();

    /**
     * @brief 设置layout是否显示
     */
    void setLayoutWidgetVisible(QLayout* layout, bool show);

    /**
     * @param btnName 指定按钮objectname
     * @param isEnterKey 是否通过键盘按键选中
     * @brief 修改按钮样式
     */
    void changeBtnState(QString btnName, bool isEnterKey = false);

    /**
     * @brief 当有inhibitor存在时显示提醒界面
     */
    void showInhibitWarning(QVector<InhibitInfo::InhibitorInfo> &list, int action);

private:
    /**
     * @brief 重新计算界面大小及控件位置布局
     */
    void ResizeEvent();

    /**
     * @brief 画出提醒界面
     */
    void drawWarningWindow(QRect &rect, QVector<InhibitInfo::InhibitorInfo> &list, int action);

    /**
     * @brief 根据inhibitor的名称获取对应desktop文件中的应用名和icon路径
     * @param allNum 共显示的按钮数
     * @param lineMaxNum 一行最多显示的按钮数
     * @param btn 指定按钮
     */
    void calculateBtnSpan(int allNum, int lineMaxNum, MyPushButton* btn, int& row, int& colum);

    /**
     * @brief 计算键盘左右键对应的按钮
     * @param key 按键名字
     */
    void calculateKeyBtn(const QString &key);

    /**
     * @brief 按钮是否可用
     * @param index 按钮序号
     */
    bool judgeBtnIsEnable(int index);

    /**
     * @brief 匹配上下按键按钮
     * @param row 行号
     * @param colum 列号
     */
    bool matchKeyBtn(int row, int colum);

    /**
     * @brief 打开监视器
     */
    void doSystemMonitor();

    /**
     * @brief 显示常规的按钮界面
     * @param hideBtnNum 隐藏的按钮数
     */
    void showNormalBtnWidget(int hideBtnNum);

    /**
     * @brief 显示需要换行的按钮界面
     * @param hideBtnNum 隐藏的按钮数
     */
    void showHasScrollBarBtnWidget(int hideBtnNum);

    void doLockscreen();//锁屏操作

    void startUkuiSplash();

Q_SIGNALS:
    /**
     * @brief 按钮点击后的事件信号
     */
    void signalTostart();

    /**
     * @brief 点击提示界面的确认按钮信号
     */
    void confirmButtonclicked();

private Q_SLOTS:
    /**
     * @brief 退出
     */
    bool exitt();

    /**
     * @brief 按键按下事件
     * @param key 键名
     */
    void onGlobalKeyPress(const QString &key);

    /**
     * @brief 按键弹起事件
     * @param key 键名
     */
    void onGlobalkeyRelease(const QString &key);

    /**
     * @brief 屏幕变化
     */
    void screenCountChanged();

    /**
     * @brief 点击按钮事件
     */
    void mouseReleaseSlots(QEvent *event, QString objName);

protected:
    void paintEvent(QPaintEvent *e);
    bool eventFilter(QObject *, QEvent *);
    void mousePressEvent(QMouseEvent *event);

public:
    int defaultnum = 0;

private:
    QPixmap pix;
    QGSettings *gs;
    QWidget *lastWidget = nullptr;
    QHash<int, MyPushButton*> map;
    QHash<int, int> rowMap;
    QHash<int, int> columMap;
    systemUpgradeCheck *upgradeCheck = nullptr;

    UkuiPower *m_power;
    XEventMonitor *xEventMonitor;
    int tableNum;
    bool flag = false;

    /// 各按钮是否隐藏 默认隐藏
    bool isSwitchuserHide = true;
    bool isHibernateHide = true;
    bool isSuspendHide = true;
    bool isLockscreenHide = true;
    bool isLogoutHide = true;
    bool isUpgradeThenRebootHide = true;
    bool isRebootHide = true;
    bool isUpgradeThenPowerOffHide = true;
    bool isPowerOffHide = true;
    bool click_blank_space_need_to_exit = true;
    bool close_system_needed_to_confirm = false;
    bool inhibitSleep = false;
    bool inhibitShutdown = false;
    bool m_isSystemMonitorSelect = false;

    QHash<MyPushButton*, bool> m_btnHideMap;

    QString m_btnImagesPath = "/usr/share/ukui/ukui-session-manager/images";
    MyPushButton *m_switchUserBtn = nullptr;
    MyPushButton *m_hibernateBtn = nullptr;
    MyPushButton *m_suspendBtn = nullptr;
    MyPushButton *m_lockScreenBtn = nullptr;
    MyPushButton *m_logoutBtn = nullptr;
    MyPushButton *m_upgradeThenRebootBtn = nullptr;
    MyPushButton *m_rebootBtn = nullptr;
    MyPushButton *m_upgradeThenshutDownBtn = nullptr;
    MyPushButton *m_shutDownBtn = nullptr;
    QWidget *m_systemMonitorBtn = nullptr;

    QLabel *m_dateLabel = nullptr;
    QLabel *m_timeLabel = nullptr;

    QLabel *m_judgeLabel = nullptr;
    CommonPushButton *m_cancelBtn = nullptr;
    CommonPushButton *m_confirmBtn = nullptr;

    QLabel *m_messageLabel1 = nullptr;
    QLabel *m_messageLabel2 = nullptr;

    QWidget *m_toolWidget = nullptr;

    QWidget *m_btnWidget = nullptr;
    QGridLayout *m_buttonHLayout = nullptr;

    QVBoxLayout *m_dateTimeLayout = nullptr;
    QVBoxLayout *m_judgeWidgetVLayout = nullptr;
    QHBoxLayout *m_judgeBtnHLayout = nullptr;
    QVBoxLayout *m_vBoxLayout = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    QHBoxLayout *m_systemMonitorHLayout = nullptr;

    QLabel *m_systemMonitorIconLabel = nullptr;
    QLabel *m_systemMonitorLabel = nullptr;
    QPixmap m_systemMonitorIcon;

    /// 计算各按钮的行列位置
    int m_switchRow = 0, m_switchColumn = 0;
    int m_hibernateRow = 0, m_hibernateColumn = 0;
    int m_suspendRow = 0, m_suspendColumn = 0;
    int m_lockScreenRow = 0, m_lockScreenColumn = 0;
    int m_logoutRow = 0, m_logoutColumn = 0;
    int m_upgradeThenRebootRow = 0, m_upgradeThenRebootColumn = 0;
    int m_rebootRow = 0, m_rebootColumn = 0;
    int m_upgradeThenShutdownRow = 0, m_upgradeThenShutdownColumn = 0;
    int m_shutDownRow = 0, m_shutDownColumn = 0;
    int m_lineNum = 1;

    QSettings *m_btnCfgSetting = nullptr;
    QRect m_screen;
    QWidget *m_showWarningArea = nullptr; /// 阻止列表
    MyListView *m_inhibitAppList = nullptr;
    QStandardItemModel *m_inhibitDataModel = nullptr;
    bool m_btnWidgetNeedScrollbar = false; /// 是否需要滑动条显示
    bool m_showWarningMesg = false; /// 是否显示阻止列表
    QVector<InhibitInfo::InhibitorInfo> m_inhibitList;
    QWidget *m_judgeWidget = nullptr; /// 提示
    int m_currentListSelect = -1;
    bool m_isFirstEnter = true;
    CommonPushButton *m_inhibitCancelBtn = nullptr;
    bool m_isFirstEnterInhibitList = true;
    bool m_isCancelBtnSelect = false;
    bool m_enterInhibitList = false;
    bool m_enterJudgeBox = false;
};
#endif // MAINWINDOW_H
