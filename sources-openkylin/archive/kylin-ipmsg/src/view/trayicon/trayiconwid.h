/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#ifndef TRAYICONWID_H
#define TRAYICONWID_H

#include <QObject>
#include <QWidget>
#include <KWindowSystem>
#include <QSystemTrayIcon>
#include <QListView>
#include <QScrollBar>
#include <QFrame>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QEvent>
#include <QTimer>
#include <QDialog>
#include <QMenu>
#include <QAction>

#include "trayitemdelegate.h"
#include "trayfiltermodel.h"

class TrayIconWid : public QWidget
{
    Q_OBJECT
    friend class TestTrayIconWid;
public:
    explicit TrayIconWid(QWidget *parent = nullptr);
    ~TrayIconWid();

    // 隐藏托盘图标
    void hideTrayIcon();

    // 生成静态实例
    static TrayIconWid *getInstance();

    // 初始化组件
    void setWidgetUi();

    // 设置组件样式
    void setWidgetStyle();

    // 设置信号绑定
    void setSignalConn();

    // 跟随主题深浅模式
    void changeTheme();

    // 窗口关闭事件
    void closeEvent(QCloseEvent *event);

signals:
    void newUnreadMsg();

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    // 系统托盘
    QSystemTrayIcon *m_trayIcon = nullptr;

    // 整体界面窗体
    QWidget *m_mainWid;

    // 聊天消息列表
    QListView *m_friendView;

    // 分割线
    QFrame *m_splitLine;

    // 功能区域
    QWidget *m_funcAreaWid;

    // 忽略消息
    QPushButton *m_ignoreBtn;

    // 整体界面布局
    QVBoxLayout *m_mainLayout;

    // 功能区域布局
    QHBoxLayout *m_funcAreaLayout;

    // 数据过滤模型
    TrayFilterModel *m_trayFilterModel;

    // 视图代理
    TrayItemDelegate *m_trayItemDelegate;

    // 右键菜单
    QMenu *m_menu = nullptr;
    QAction *m_settingAction = nullptr;
    QAction *m_closeAction = nullptr;

    // 任务栏图标闪烁
    int m_timeFlag;
    QTimer *m_alertTimer;
    QIcon m_themeIcon;
    QIcon m_blankIcon;

    // 消息数
    int m_msgCounter;

    // 开始图标闪烁
    void slotStartAlert();

    // 图标闪烁
    void slotMsgAlert();

    // 事件过滤器
    bool eventFilter(QObject *watch, QEvent *event);

    // 绘制事件
    void paintEvent(QPaintEvent *event);

    // 托盘图标事件
    void iconActivated(QSystemTrayIcon::ActivationReason reason);

    // 发起聊天
    void slotStartChat();

    // 忽略全部消息
    void slotIgnoreAll();

    // 右键任务栏的关闭
    void slotCloseWin();

    // 右键任务栏的设置
    void slotSetting();
};




#endif // TRAYICONWID_H
