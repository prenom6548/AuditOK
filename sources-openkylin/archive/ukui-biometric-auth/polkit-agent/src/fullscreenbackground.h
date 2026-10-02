/*
 * Copyright (C) 2024 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef FULLSCREENMANAGER_H
#define FULLSCREENMANAGER_H

#include <QWidget>
#include <QList>
#include <QPair>
#include <QRect>
#include <PolkitQt1/ActionDescription>
#include <QPixmap>
#include <QScreen>
#include "usdblockshortcut.h"
#include <ukuiwindowhelper.h>

class MainWindow;
class FullScreenBackground : public QWidget
{
    Q_OBJECT
public:
    explicit FullScreenBackground(QWidget *parent = nullptr);
    virtual ~FullScreenBackground();

    /**
     * @brief getMainWindow 获取认证对话框
     * @return 认证对话框指针
     */
    inline MainWindow *getMainWindow()
    {
        return m_wndMain;
    }

    /**
     * @brief initMainWindow 初始化认证对话框
     * @param strIconName 认证图标名称
     * @param strMsg 认证消息
     * @param listUsers 认证用户列表
     * @param subjectPid 项目进程号
     * @param callerPid 调用者进程号
     * @param actionDesc 动作描述
     * @param isMavis 是否为mavis平台
     * @param isSupportTableMode 是否支持平板模式
     */
    void initMainWindow(
        const QString &strIconName,
        const QString &strMsg,
        const QStringList &listUsers,
        const QString &subjectPid,
        const QString &callerPid,
        const PolkitQt1::ActionDescription &actionDesc,
        const bool &isMavis,
        const bool &isSupportTableMode);

    /**
     * @brief showDialog 显示认证对话框
     */
    void showDialog();

    /**
     * @brief closeDialog 关闭认证对话框
     */
    void closeDialog();

protected:
    /**
     * @brief paintEvent 绘制事件处理
     * @param event 事件指针
     */
    void paintEvent(QPaintEvent *event);

    /**
     * @brief closeEvent 窗口关闭事件处理
     * @param event 事件指针
     */
    void closeEvent(QCloseEvent *event);

    /**
     * @brief resizeEvent 窗口大小改变事件处理
     * @param event 事件指针
     */
    void resizeEvent(QResizeEvent *event);

    bool eventFilter(QObject *obj, QEvent *event);

private Q_SLOTS:
    /**
     * @brief onScreenCountChanged 屏幕数变化处理
     */
    void onScreenCountChanged(int);

    /**
     * @brief onDesktopResized 桌面大小变化处理
     */
    void onDesktopResized();

    /**
     * @brief onPrimaryScreenChanged 主屏变化处理
     */
    void onPrimaryScreenChanged();

    /**
     * @brief onWorkAreaResized 工作区大小变化处理
     */
    void onWorkAreaResized();

    void onCurrentScreenChanged(QScreen *screen);
private:
    /**
     * @brief initUI 初始化UI
     */
    void initUI();

    /**
     * @brief initConnections 初始化信号槽连接
     */
    void initConnections();
    /**
     * @brief isSupportCompositing 是否支持窗口复合
     * @return true 支持，否则不支持
     */
    bool isSupportCompositing();

    /**
     * @brief getBackground 获取背景
     * @return 背景pixmap
     */
    QPixmap getBackground();

    /**
     * @brief getAccountBackground 获取账户背景
     * @return 背景路径
     */
    QString getAccountBackground();

    /**
     * @brief moveToScreen 移动认证弹窗到指定屏幕
     * @param screen 屏幕信息指针
     */
    void moveToScreen(QScreen *screen);

    /**
     * @brief moveToCursorScreen 移动认证窗口到光标屏幕
     */
    void moveToCursorScreen();

    /**
     * @brief blockSomeShortCut 禁启用快捷键
     * @param isBlock 是否为禁用
     */
    void blockSomeShortCut(bool isBlock = false);

private:
    MainWindow *m_wndMain = nullptr;                // 认证对话框
    QPixmap m_pixmapBackground;                     // 背景图
    bool m_isFirstShow = true;                      // 第一次显示才主动移动对话框居中
    USDBlockShortCut *m_usdBlockShortCut = nullptr; // usd禁启用快捷键实例
    bool m_isWayland = false;
    UkuiWindowHelper *m_windowHelper = nullptr;
    QString m_curScreenName = "";
};

#endif // FULLSCREENMANAGER_H
