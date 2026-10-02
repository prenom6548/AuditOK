/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QList>
#include <QObject>
#include <QDir>
#include <QProgressBar>
#include <QScrollArea>
#include <QFrame>
#include <QThread>
#include "kom-label.h"
#include "item_widget.h"
#include "nm_dbus_adaptor.h"
#include "libBase.h"
#include "hwcheck.h"
#include "ipcheck.h"
#include "dhcpcheck.h"
#include "dnscheck.h"
#include "hostcheck.h"
#include "netcheck.h"
#include "quad_btns_title_bar.h"
#include "config_win.h"
#include "observer.h"
#include "custom_label.h"
#include "kprogressbar.h"

using namespace kdk;

namespace Ui
{
class MainWindow;
}
//封装了signal的观察者
class PluginsObserver : public QObject, public Observer
{
    Q_OBJECT
public:
    PluginsObserver() {}
signals:
    void updateMainWinUI(const statusStruct &);

public:
    void update(const statusStruct &curStatus)
    {
        emit updateMainWinUI(curStatus);
    }
};

//插件结构体，将ui，观察者，插件三者对应上管理。
struct PluginKits
{
    LibBase *m_pluginPtr = nullptr;
    ItemWidget *m_UIPtr = nullptr;
    PluginsObserver *m_pluginObserver = nullptr;
    //完成初始化，建立 通知-观察者 关系
    void setInit(LibBase *ptr)
    {
        m_pluginPtr = ptr;
        m_UIPtr = new ItemWidget;
        m_pluginObserver = new PluginsObserver;
        m_pluginPtr->add(m_pluginObserver);
    }
    ~PluginKits()
    {
        if (m_UIPtr) {
            delete m_UIPtr;
            m_UIPtr = nullptr;
        }
        if (m_pluginObserver) {
            delete m_pluginObserver;
            m_pluginObserver = nullptr;
        }
    }
};

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    //刷新主界面各检查项部分UI
    void refreshUI();
    void changeThemeColor(int color);
    void changeSystemSize(int size);
public slots:
    //显示设置窗口
    void showConfigureWin();
    //显示内网检测详情
    void showContent();
    //根据内网检测结果，刷新详情区域内容
    void refreshInnerCheckRes(QMap<QString, QMap<QString, bool>> resMap);
    //主窗口插件部分UI更新
    void updateMainWinUISlot(const statusStruct &curStatus);
    //从第几项开始检测（为后续暂停检测，又继续检测的接口）
    void pluginStartCheck(int);
    void slotUpdateConfigFile();
    //开始检查函数
    void startCheckProcess();
private slots:
    //手动停止
    void manualStopCheckProcess();
    //重新检测槽函数
    void reCheckProcess();
    //全部检测项检测完毕后进行主界面标题区域刷新操作
    void allFinished();
    //因错误停止检测
    void errStopCheckProcess();
    //当前停止项收到手动停止的信号
    void slotManualStopCheck();
    void slotReturnBtnClick();
signals:
    void startCheckIndex(int index);
    void manualStoped();
    void sigChangeStackWidgetMain(int page);

private:
    QPushButton *cancelBtn = nullptr;
    QPushButton *checkAgainBtn = nullptr;
    QPushButton *startCheckBtn = nullptr;
    QPushButton *returnBtn = nullptr;
    kom::KomLabel *contentTitleLabel = nullptr;
    CustomLabel *statusLabel = nullptr;
    QPushButton *mainIcon = nullptr;
    QFrame *line = nullptr;
    KProgressBar *progressBar = nullptr;
    QScrollArea *scrollArea = nullptr;
    QWidget *scrollAreaWidgetContents = nullptr;
    QVBoxLayout *allView = nullptr;
    QWidget *checkWidget = nullptr;
    QVBoxLayout *widgetView = nullptr;
    CustomLabel *mTitleLabel = nullptr;
    CustomLabel *mItemSumLabel = nullptr;
    //读取插件
    void loadingPlugins();
    //首次初始化UI界面
    void initUI();
    //初始化DBUS
    void initDBus();
    //初始化gsettings
    void initSettings();
    // SP1定制，没有主题控件，需要自己写stylesheet
    void themeUIRefresh();
    //插件结构体list增加成员函数
    void pluginKitsIncrease();
    void setProgressBarVisible(bool isBarShow);

    //用于最小化拉起的dbus，暂未使用
    NmDbusAdaptor *m_NWMDBus = nullptr;
    //是否继续检测flag
    bool m_goOnCheck = false;
    QDir m_pluginsDir;
    //读取出的内网检测配置
    InnerNetCheck m_innerCheckArgSettings;
    //读取出的关于内网检测的gsettings配置
    QGSettings *m_checkSettings = nullptr;
    //读取出的关于主题的gsettings配置
    QGSettings *m_themeSettings = nullptr;
    //当前主题theme
    QString m_curStyle = "ukui-default";
    //右上角三联按钮
    QuadBtnsTitleBar *m_quadsBtn;
    //网络检测结果
    bool m_isNetAllRight = true;
    //显示详情区域ui list
    QList<ItemWidget *> m_variableItems;
    //显示详情区域ip、web标题
    ItemWidget *m_ipTitle = nullptr;
    ItemWidget *m_webTitle = nullptr;
    bool isShow = false;
    int themeColor = 0;
    int mFontSize = 0;

    void setCheckResultDesc(void);

    // 存储加载进来的插件信息，插件索引与元素在 List 中的位置强绑定
    QList<PluginKits> m_pluginList;
    int m_correctItem = 0;
    int m_warningItem = 0;
    int m_errorItem = 0;
};

#endif // MAINWINDOW_H
