/*
 *
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef UKUITASKMANAGER_H
#define UKUITASKMANAGER_H

#include <QObject>
#include <QAbstractListModel>
#include <memory>
#include <QQuickItem>

#include "actions.h"

namespace TaskManager {

class UkuiTaskManager : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString currentActiveWinId READ currentActiveWinId NOTIFY currentActiveWinIdChanged)
    Q_PROPERTY(bool disableMouseLeftButton READ disableMouseLeftButton NOTIFY disableMouseLeftButtonChanged)
    Q_PROPERTY(bool disableMouseRightButton READ disableMouseRightButton NOTIFY disableMouseRightButtonChanged)
    Q_PROPERTY(bool disableMouseHover READ disableMouseHover NOTIFY disableMouseHoverChanged)
    Q_PROPERTY(bool enableDragToApp READ enableDragToApp WRITE setEnableDragToApp NOTIFY enableDragToAppChanged)
    Q_PROPERTY(int mergeIcons READ mergeIcons NOTIFY mergeIconsChanged)

public:
    enum AdditionalRoles {
        Id = Qt::UserRole + 1, //desktop文件全路径
        DesktopFile,
        Name,                  //名称
        GenericName,           //应用GenericName
        IconName,              //图标名称
        Icon,                  //图标
        WinIdList,               //winID列表
        CurrentDesktopWinIdList, //当前工作区的窗口
        WindowGeometries,
        WindowTitles,
        WindowIcons,
        WindowIconsName,
        CurrentWinIdList,
        CurrentScreenWinIdList, //当前屏幕上的窗口列表
        DemandsAttentionWinIdList,  //处于demandsAttention状态的窗口列表
        HasLauncher,    //快速启动按钮位置（从0开始），-1表示无快速启动按钮
        Actions,               //可执行的action，目前只包含desktop action
        UnreadMessagesNum,     //未读消息数量
        HasActiveWindow,
        ApplicationMenuServiceName,
        ApplicationMenuObjectPath,
        QuickWidgetId
    };
    Q_ENUM(AdditionalRoles)
    /**
     * 多屏时，任务栏图标显示在哪个任务栏 枚举值
     * 0-所有屏幕，1-主屏和打开了窗口的任务栏 2-只显示在打开了窗口的任务栏
     */
    enum TaskBarIconsShowedOn {
        AllPanels = 0,
        PrimaryScreenPanelAndPanelWhereWindowIsOpen,
        PanelWhereWindowIsOpen
    };
    Q_ENUM(TaskBarIconsShowedOn)
    /**
     * 是否合并任务栏上的图标
     * 0-始终合并，1-从不合并
     */
    enum MergeIcons {
        Always = 0,
        Never
    };
    Q_ENUM(MergeIcons)
    static UkuiTaskManager &self();
    ~UkuiTaskManager() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    int rowCount(const QModelIndex &parent) const override;

    /**
     * 快速启动按钮 增加/交换位置
     * @param desktopFile 目标desktop文件
     * @param order 增加或交换位置
     */
    void setQuickLauncher(const QString &desktopFile, int order);
    void setOrder(const QModelIndex &index, int order);
    QModelIndex indexOf(const QString &appId);
    QString currentActiveWinId() const;

    Q_INVOKABLE static void activateWindowView(const QStringList &winIds);
    /**
     * 激活窗口
     * @param winId
     */
    Q_INVOKABLE void activateWindow(const QString &winId);
    /**
     * 窗口标题
     * @param winId
     * @return
     */
    Q_INVOKABLE QString windowTitle(const QString &winId);
    /**
     * 窗口图标
     * @param winId
     * @return
     */
    Q_INVOKABLE QIcon windowIcon(const QString &winId);
    /**
     * 窗口action
     * @param winId
     * @return
     */
    Q_INVOKABLE TaskManager::Actions windowActions(const QString &winId);
    /**
     * 窗口是否被激活
     * @param winId
     * @return bool
     */
    Q_INVOKABLE static bool windowIsActivated(const QString &winId);
    /**
    * 窗口执行指定动作
    * @param Action::Type
    * @param winId
    */
    Q_INVOKABLE static void execSpecifiedAction(const TaskManager::Action::Type &type, const QString &wid);
    Q_INVOKABLE TaskManager::Actions menuActions(const QModelIndex &index);
    void launch(const QModelIndex &index, const QQuickItem *item = nullptr);
    bool event(QEvent* ev) override;
    void requestDataChange();

    bool disableMouseLeftButton() const;
    bool disableMouseRightButton() const;
    bool disableMouseHover() const;
    int mergeIcons() const;
    QStringList appWhiteList() const;
    QStringList appBlackList() const;
    bool enableDragToApp() const;
    void setEnableDragToApp(bool enable);
    int taskbarIconsShowedOnByControl() const;

Q_SIGNALS:
    void unReadMessagesNumberUpdate(const QString &desktopFile, uint num);
    void currentActiveWinIdChanged();
    void disableMouseLeftButtonChanged();
    void disableMouseRightButtonChanged();
    void disableMouseHoverChanged();
    void mergeIconsChanged();
    void appWhiteListChanged();
    void appBlackListChanged();
    void enableDragToAppChanged();
    void taskbarIconsShowedOnChanged();

public Q_SLOTS:
    /*
     * 这四个接口是对外的接口，用于添加/删除/检查/获取快速启动按钮
     */
    bool addQuickLauncher(const QString &desktopFile);
    bool checkQuickLauncher(const QString &desktopFile);
    bool removeQuickLauncher(const QString &desktopFile);
    QStringList quickLaunchers() const;

    static QString standardizePath(const QString &path);

    void closeWindow(const QVariant& wid);
    void minimizeWindow(const QVariant& wid);
    void maximizeWindow(const QVariant& wid);
    void restoreWindow(const QVariant& wid);
    void keepAbove(const QVariant& wid);
    void unsetKeepAbove(const QVariant& wid);

private:
    explicit UkuiTaskManager(QObject *parent = nullptr);
    /**
     * 添加快捷按钮到指定位置（新增、交换位置）
     * @param desktopFile
     * @param order 图标在m_items对应的位置
     */
    void addQuickLauncher(const QString &desktopFile, int order);
    int quickLauncherCountBeforeRow(int row) const;
    void onAppUninstalled(const QStringList &desktopFiles);
    /**
     * 固定到任务栏/取消固定操作上传埋点数据
     * @param applicationName 目标desktop文件
     */
    void addQuickLauncherEvent(const QString &applicationName, const QString &operate);
    void onFunctionControlChanged(const QString& name, bool exit = false);
    class Private;
    Private *d = nullptr;
};
}
#endif // UKUITASKMANAGER_H
